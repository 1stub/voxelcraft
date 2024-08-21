#include "chunk.h"
#include "block.h"

Chunk::Chunk(int xOff, int zOff, const siv::PerlinNoise &p) : xOffset(xOff), zOffset(zOff){
  //set blocks
  generateHeightMap(p);
  setBlockTexture();
  for(int x = 0; x < chunkSize; x++){
    for(int z = 0; z < chunkSize; z++){
      double noiseValue = getNoiseValue(p, x, z);
      int height = static_cast<int>(noiseValue) + chunkSize;
      for(int y = 0; y < chunkHeight; y++){
        if (voxelGrid[x+1][y][z+1] == 1) {
          Block b((xOffset * chunkSize) + x, y, (zOffset * chunkSize) + z);
          b.setType(y == height - 1 ? Grass : y > height - 4 ? Dirt : Stone);

          if (checkNeighbors(b, x + 1, y, z + 1)) {
              blocks.push_back(b);
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
    return p.normalizedOctave2D_01(((xOffset * chunkSize) + x) * frequency, ((zOffset * chunkSize) + z) * frequency, octaves, persistence) * (double)25;
}

void Chunk::generateHeightMap(const siv::PerlinNoise &p){
    for(int x = -1; x < chunkSize + 1; x++){
        for(int z = -1; z < chunkSize + 1; z++){
            double noiseValue = getNoiseValue(p, x, z);
            int height = static_cast<int>(noiseValue) + chunkSize;
            for(int y = 0; y < chunkHeight; y++){
                voxelGrid[x+1][y][z+1] = (y < height) ? 1 : 0;
            }
        }
    }
}

int Chunk::getNumBlocks(){
  return blocks.size();
}

void Chunk::initChunk(){
  int totalSize = 0;
  for(const auto &b : blocks){
    totalSize += b.vertices.size();
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
  for(const auto &b : blocks){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, offset, b.vertices.size() * sizeof(float), &b.vertices.front());
    offset += b.vertices.size() * sizeof(float);
  }

  //make this call once, textures get created based on the coords associated with the block
  textureBlocks();
}

//This checks to see what faces are visible
//Will eventually need to use the voxelGrid for this
bool Chunk::checkNeighbors(Block &b, int x, int y, int z) {
    blockTexCoords b_texSides = blockTextures[b.getBlockId()];
    blockTexCoords b_texTop = blockTextures[b.getBlockId()];
    blockTexCoords b_texBottom = blockTextures[b.getBlockId()];

    if(b.getBlockId() == Grass) {
        b_texTop = blockTextures[GrassTop];
        b_texBottom = blockTextures[Dirt];
    }

    bool faceDrawn = false;

    // Helper lambda to check if the block is air (or not solid)
    auto isAir = [&](int x, int y, int z) -> bool {
        // Check if coordinates are within the chunk
        if (x >= 0 && x < chunkSize + 2 && z >= 0 && z < chunkSize + 2 && y > 0 && y < chunkHeight) {
            return voxelGrid[x][y][z] == 0;
        }
        // If out of bounds, check neighboring chunks (this part depends on how you store/access neighboring chunks)
        // For example:
        // if (x < 0) return getNeighboringChunk(LEFT)->isAir(x + chunkSize, y, z);
        // if (x >= chunkSize + 2) return getNeighboringChunk(RIGHT)->isAir(x - chunkSize, y, z);
        // if (z < 0) return getNeighboringChunk(FRONT)->isAir(x, y, z + chunkSize);
        // if (z >= chunkSize + 2) return getNeighboringChunk(BACK)->isAir(x, y, z - chunkSize);

        // for this program I am not accessing neigboring chunks but rather storing padding around my voxel map.

        return false;  // Assume out of bounds means air
    };

    // Above
    if (isAir(x, y + 1, z)) {
        b.insertVertices(topFace, b_texTop);
        faceDrawn = true;
    }

    // Below
    if (isAir(x, y - 1, z)) {
        b.insertVertices(bottomFace, b_texBottom);
        faceDrawn = true;
    }

    // Right
    if (isAir(x + 1, y, z)) {
        b.insertVertices(rightFace, b_texSides);
        faceDrawn = true;
    }

    // Left
    if (isAir(x - 1, y, z)) {
        b.insertVertices(leftFace, b_texSides);
        faceDrawn = true;
    }

    // Front
    if (isAir(x, y, z - 1)) {
        b.insertVertices(frontFace, b_texSides);
        faceDrawn = true;
    }

    // Behind
    if (isAir(x, y, z + 1)) {
        b.insertVertices(backFace, b_texSides);
        faceDrawn = true;
    }

    return faceDrawn;
}

// decided to try to use Bresenham Line Algo instead 
//glm::vec3 Chunk::checkRayIntersection(Raycast &ray, Camera &c){
//}

void Chunk::drawChunk(){
  glDisable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, verticeCount);
  glBindVertexArray(0);
  glEnable(GL_CULL_FACE);
}
