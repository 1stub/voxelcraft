#include "chunk.h"
#include "block.h"
#include <memory>

Chunk::Chunk(int xOff, int zOff, const siv::PerlinNoise &p) : xOffset(xOff), zOffset(zOff){
  //set blocks
  generateHeightMap(p);
  setBlockTexture();
  for(int x = 0; x < Chunks::size; x++){
    for(int z = 0; z < Chunks::size; z++){
      double noiseValue = getNoiseValue(p, x, z);
      int iNoiseVal = noiseValue < 0.0 ? glm::ceil(noiseValue) : glm::floor(noiseValue);
      int height = iNoiseVal + Chunks::size;
      for(int y = 0; y < Chunks::height; y++){
        if (voxelGrid[x+1][y][z+1] == 1) {
          //our block position
          glm::ivec3 bPos((xOffset * Chunks::size) + x, y, (zOffset * Chunks::size) + z);
          
          //stores a vector containing integers corresponding to face to draw
          std::vector<int>faces = checkNeighbors(x + 1, y, z + 1);
          
          if (faces.size() > 0) {
            blocks.emplace(bPos, std::make_unique<Block>(bPos.x, bPos.y, bPos.z));
            blocks[bPos]->setType(y == height - 1 ? Grass : y > height - 4 ? Dirt : Stone);
            setFaces(bPos, faces);
          }
        }
      }
    }
  }
  initChunk();
  updateVertices();
}

//This creates a heightmap we use to check adjacent blocks
//created padding around map to allow for adjacent blocks in other chunks to be visible for determinng renderd faces
double Chunk::getNoiseValue(const siv::PerlinNoise &p, int x, int z) {
    return p.normalizedOctave2D_01(((xOffset * Chunks::size) + x) * frequency, ((zOffset * Chunks::size) + z) * frequency, octaves, persistence) * (double)25;
}

void Chunk::generateHeightMap(const siv::PerlinNoise &p){
    for(int x = -1; x < Chunks::size + 1; x++){
        for(int z = -1; z < Chunks::size + 1; z++){
            double noiseValue = getNoiseValue(p, x, z);
            int height = static_cast<int>(noiseValue) + Chunks::size;
            for(int y = 0; y < Chunks::height; y++){
                voxelGrid[x+1][y][z+1] = (y < height) ? 1 : 0;
            }
        }
    }
}

std::shared_ptr<Block> Chunk::fetchBlock(glm::ivec3 blockCoords){
  auto block = blocks[blockCoords];
  return block;
}

int Chunk::getNumBlocks(){
  return blocks.size();
}

void Chunk::deleteBlock(glm::ivec3 voxel, const siv::PerlinNoise &p, std::vector<Chunk*> adjChunks) {
  // Normalize voxel coordinates into chunk-local coordinates (0-15 range)
  glm::ivec3 normalizedBlockCoords(
      (voxel.x % Chunks::size + Chunks::size) % Chunks::size, // Handles negative modulo correctly
      voxel.y,
      (voxel.z % Chunks::size + Chunks::size) % Chunks::size  // Handles negative modulo correctly
  );
  voxelGrid[normalizedBlockCoords.x + 1][normalizedBlockCoords.y][normalizedBlockCoords.z + 1] = 0;
  //if we are on boundary of chunk, we need adjacent
  //think this needs to be in the chunkManager so I can easly access adjacent chunk from map

  blocks.erase(voxel);

  std::vector<glm::ivec3> directions = {
      {0, 1, 0},   // Above
      {0, -1, 0},  // Below
      {-1, 0, 0},  // Left
      {1, 0, 0},   // Right
      {0, 0, -1},  // Behind
      {0, 0, 1}    // In front
  };
  // Iterate through directions to check adjacent blocks
  for (const auto& direction : directions) {
      glm::ivec3 neighborPos = voxel + direction;
      glm::ivec3 normalizedNeighborPos = normalizedBlockCoords + direction;

      // Check if the neighbor block is within the current chunk
      bool isWithinChunk = (normalizedNeighborPos.x >= 0 && normalizedNeighborPos.x < Chunks::size &&
                            normalizedNeighborPos.z >= 0 && normalizedNeighborPos.z < Chunks::size);

      // Check if the neighbor block is part of the current chunk
      if (isWithinChunk) {
          if (voxelGrid[normalizedNeighborPos.x + 1][normalizedNeighborPos.y][normalizedNeighborPos.z + 1] == 1) {
              double noiseValue = getNoiseValue(p, normalizedNeighborPos.x, normalizedNeighborPos.z);
              int iNoiseVal = noiseValue < 0.0 ? glm::ceil(noiseValue) : glm::floor(noiseValue);
              int height = iNoiseVal + Chunks::size;

              std::vector<int> faces = checkNeighbors(normalizedNeighborPos.x + 1, normalizedNeighborPos.y, normalizedNeighborPos.z + 1);

              // If the neighbor block does not exist and new faces are needed, create a new block
              if (blocks.find(neighborPos) == blocks.end() && !faces.empty()) {
                  blocks.emplace(neighborPos, std::make_unique<Block>(neighborPos.x, neighborPos.y, neighborPos.z));
                  blocks[neighborPos]->setType(neighborPos.y == height - 1 ? Grass : neighborPos.y > height - 4 ? Dirt : Stone);
                  setFaces(neighborPos, faces);
              } else if (!faces.empty()) {  // Update existing block faces if necessary
                  setFaces(neighborPos, faces);
              }
          }
      } else {
          // Handle cases where the neighboring block is in an adjacent chunk
        glm::ivec2 neighborChunkCoords(
          neighborPos.x >= 0 ? neighborPos.x / Chunks::size : (neighborPos.x - Chunks::size + 1) / Chunks::size,
          neighborPos.z >= 0 ? neighborPos.z / Chunks::size : (neighborPos.z - Chunks::size + 1) / Chunks::size
        );

        for( auto c : adjChunks){
          if(c->xOffset == neighborChunkCoords.x || c->zOffset == neighborChunkCoords.y){ 
            c->updateChunkOnBlockBreak(neighborPos, voxel);
          }
        }
      }
  }

  // Modify buffer with new data
  updateVertices();
}

//the idea here is to look across chunk boundaries and find if the block adjacent to what was broken is solid
//if it is we need to regen its buffers based on what is exposed then update the vbo
void Chunk::updateChunkOnBlockBreak(const glm::ivec3 blockPos, const glm::ivec3 originalBlockPos){
  const glm::ivec3 normalizedBlockPos(
    (blockPos.x % Chunks::size + Chunks::size) % Chunks::size, // Handles negative modulo correctly
    blockPos.y,
    (blockPos.z % Chunks::size + Chunks::size) % Chunks::size  // Handles negative modulo correctly
  );
  const glm::ivec3 normalizedOriginalBlockPos(
    (originalBlockPos.x % Chunks::size + Chunks::size) % Chunks::size, // Handles negative modulo correctly
    originalBlockPos.y,
    (originalBlockPos.z % Chunks::size + Chunks::size) % Chunks::size  // Handles negative modulo correctly
  );
  //update chunks voxelGrid with new deleted Block
  //what I was trying to do was properly update the DELETED block not the block we are looking at based on direction from delete block function
  /*if(normalizedOriginalBlockPos.x == 0) voxelGrid[0][normalizedOriginalBlockPos.y][normalizedOriginalBlockPos.z + 1] = 0;
  if(normalizedOriginalBlockPos.x == Chunks::size - 1) voxelGrid[normalizedOriginalBlockPos.x + 1][normalizedOriginalBlockPos.y][Chunks::size + 1] = 0;
  if(normalizedOriginalBlockPos.z == 0) voxelGrid[normalizedOriginalBlockPos.x + 1][normalizedOriginalBlockPos.y][Chunks::size + 1] = 0;
  if(normalizedOriginalBlockPos.x == Chunks::size - 1) voxelGrid[0][normalizedOriginalBlockPos.y][normalizedOriginalBlockPos.z + 1] = 0;*/

  if(normalizedBlockPos.x == 0) voxelGrid[0][normalizedBlockPos.y][normalizedBlockPos.z + 1] = 0;
  if(normalizedBlockPos.x == Chunks::size - 1) voxelGrid[Chunks::size + 1][normalizedBlockPos.y][normalizedBlockPos.z + 1] = 0;
  if(normalizedBlockPos.z == 0) voxelGrid[normalizedBlockPos.x + 1][normalizedBlockPos.y][0] = 0;
  if(normalizedBlockPos.x == Chunks::size - 1) voxelGrid[normalizedBlockPos.x + 1][normalizedBlockPos.y][Chunks::size + 1] = 0;

  std::vector<int>faces = checkNeighbors(normalizedBlockPos.x + 1, normalizedBlockPos.y, normalizedBlockPos.z + 1);
  
  if (voxelGrid[normalizedBlockPos.x + 1][normalizedBlockPos.y][normalizedBlockPos.z + 1] == 1 && blocks.find(blockPos) != blocks.end() && faces.size() > 0) {
    blockType bT = blocks[blockPos]->getBlockType();
    blocks.erase(blockPos);
    blocks.emplace(blockPos, std::make_unique<Block>(blockPos.x, blockPos.y, blockPos.z));
    blocks[blockPos]->setType(bT);
    setFaces(blockPos, faces);
  }

  updateVertices();
}

//will need to modify to remove face between blocks next to and current block being places
void Chunk::placeBlock(const glm::ivec3 voxel){
  glm::ivec3 newBlock(voxel.x, voxel.y + 1, voxel.z);
  glm::ivec3 normalizedBlockCoords(
      (newBlock.x % Chunks::size + Chunks::size) % Chunks::size, // Handles negative modulo correctly
      newBlock.y,
      (newBlock.z % Chunks::size + Chunks::size) % Chunks::size  // Handles negative modulo correctly
  );
  voxelGrid[normalizedBlockCoords.x+1][normalizedBlockCoords.y][normalizedBlockCoords.z + 1] = 1;
  std::vector<int> faces = checkNeighbors(normalizedBlockCoords.x + 1, normalizedBlockCoords.y, normalizedBlockCoords.z + 1);
  blocks.emplace(newBlock, std::make_unique<Block>(newBlock.x, newBlock.y, newBlock.z));
  blocks[newBlock]->setType(Stone);
  setFaces(newBlock, faces);
  updateVertices();
}

std::vector<glm::ivec3> Chunk::getBlocks(){
  std::vector<glm::ivec3> ret;
  for(auto &b : blocks){
    ret.push_back(glm::ivec3(b.second->x, b.second->y, b.second->z));
  }
  return ret;
}

void Chunk::initChunk(){
  int totalSize = 0;
  for(auto &b : blocks){
    totalSize += b.second->vertices.size();
  }
  verticeCount = totalSize;

  glGenTextures(1, &texture);
  
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);  
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, totalSize * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
  
  //position attrib
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // texture attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float))); //we have to offset by 3*sizeof(float);
  glEnableVertexAttribArray(1);
}

//we need to update our vbo with new vertex data
void Chunk::textureBlocks() {
  glBindTexture(GL_TEXTURE_2D, texture);
  
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 7); // pick mipmap level 7 or lower

  // load and generate the texture
  int width, height, nrChannels;
  unsigned char *data = stbi_load("../img/atlas.png", &width, &height, &nrChannels, 0);
  if (data)
  {
    //stbi_set_flip_vertically_on_load(1);
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
      std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}

//this generates our textures into an array - should probably be done in the chunkManager
void Chunk::setBlockTexture(){
  constexpr float y = 0;
  constexpr float sheetWidth = 64.0f;
  constexpr float sheetHeight = 64.0f;
  constexpr float spriteWidth = 16.0f , spriteHeight = 16.0f;
  
  for(float x = 0; x < 4; x++){
    blockTextures[static_cast<int>(x)] = {
      { (x * spriteWidth) / sheetWidth, (y * spriteHeight) / sheetHeight}, //topleft
      { ((x + 1) * spriteWidth) / sheetWidth, (y * spriteHeight) / sheetHeight}, //top right
      { ((x + 1) * spriteWidth) / sheetWidth, ((y + 1) * spriteHeight) / sheetHeight}, //bottomright
      { ((x + 1) * spriteWidth) / sheetWidth, ((y + 1) * spriteHeight) / sheetHeight}, //bottom right
      { (x * (spriteWidth)) / sheetWidth, ((y + 1) * spriteHeight) / sheetHeight}, //bottom left
      { (x * spriteWidth) / sheetWidth, (y * spriteHeight) / sheetHeight} //topleft
    };
  }
}

//we need to update our vbo with new vertex data
void Chunk::updateVertices() {
    int totalSize = 0;
    std::vector<float> data;
    for (auto &b : blocks) {
        totalSize += b.second->vertices.size();
        data.insert(data.end(), b.second->vertices.begin(), b.second->vertices.end());
    }
    verticeCount = totalSize;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);

    // Optionally, bind the VAO again if required by your rendering process.
    //glBindVertexArray(VAO);

    // Ensure the texture setup is called correctly (if needed).
    textureBlocks();

    // Unbind the VBO (optional, for safety).
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//This checks to see what faces are visible
//Will eventually need to use the voxelGrid for this
std::vector<int> Chunk::checkNeighbors(int x, int y, int z){
    std::vector<int> faces;
    // Helper lambda to check if the block is air (or not solid)
    auto isAir = [&](int x, int y, int z) -> bool {
        // Check if coordinates are within the chunk
        if (x >= 0 && x < Chunks::size + 2 && z >= 0 && z < Chunks::size + 2 && y > 0 && y < Chunks::height) {
            return voxelGrid[x][y][z] == 0;
        }
        return false;  // Assume out of bounds means air
    };

    //Above = 0
    //Below = 1
    //Right = 2
    //Left = 3
    //Front = 4
    //Behind = 5

    // Above
    if (isAir(x, y + 1, z)) {
        faces.push_back(0);
    }

    // Below
    if (isAir(x, y - 1, z)) {
        faces.push_back(1);
    }

    // Right
    if (isAir(x + 1, y, z)) {
        faces.push_back(2);
    }

    // Left
    if (isAir(x - 1, y, z)) {
        faces.push_back(3);
    }

    // Front
    if (isAir(x, y, z - 1)) {
        faces.push_back(4);
    }

    // Behind
    if (isAir(x, y, z + 1)) {
        faces.push_back(5);    
    }
    return faces;
}

void Chunk::setFaces(glm::ivec3 bPos, std::vector<int> faces){
  blockTexCoords b_texSides = blockTextures[blocks[bPos]->getBlockId()];
  blockTexCoords b_texTop = blockTextures[blocks[bPos]->getBlockId()];
  blockTexCoords b_texBottom = blockTextures[blocks[bPos]->getBlockId()];

  if(blocks[bPos]->getBlockId() == Grass) {
      b_texTop = blockTextures[GrassTop];
      b_texBottom = blockTextures[Dirt];
  }

  for(int f : faces){
    if(f == 0){
      blocks[bPos]->insertVertices(topFace, b_texTop);
    }
    if(f == 1){
      blocks[bPos]->insertVertices(bottomFace, b_texBottom);
    }
    if(f == 2){
      blocks[bPos]->insertVertices(rightFace, b_texSides);
    }
    if(f == 3){
      blocks[bPos]->insertVertices(leftFace, b_texSides);
    }
    if(f == 4){
      blocks[bPos]->insertVertices(frontFace, b_texSides);
    }
    if(f == 5){
      blocks[bPos]->insertVertices(backFace, b_texSides);
    }
  }
}

void Chunk::drawChunk(){
  glDisable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, verticeCount);
  glBindVertexArray(0);
  glEnable(GL_CULL_FACE);
}
