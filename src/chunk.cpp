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
      int height = static_cast<int>(noiseValue) + Chunks::size;
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
void Chunk::updateVertices(){
  int offset = 0;
  for(auto &b : blocks){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, offset, b.second->vertices.size() * sizeof(float), &b.second->vertices.front());
    offset += b.second->vertices.size() * sizeof(float);
  }

  //make this call once, textures get created based on the coords associated with the block
  textureBlocks();
}

//This checks to see what faces are visible
//Will eventually need to use the voxelGrid for this
std::vector<int> Chunk::checkNeighbors(int x, int y, int z) {
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
