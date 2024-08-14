#include "chunk.h"

Chunk::Chunk(){
  //set blocks
  generateVoxelGrid();
  for(int x = 0; x < chunkSize; x++){
    for(int z = 0; z < chunkSize; z++){
        for(int y = 0; y < chunkSize; y++){ 
        Block b(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        if(checkNeighbors(b, x, y, z)){
          blocks.push_back(b);
        }
      }
    }
  }
  initChunk();
  updateVertices();
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
  glBindTexture(GL_TEXTURE_2D, texture);
  
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

  textureBlocks();
}

//we need to update our vbo with new vertex data
void Chunk::updateVertices(){
  int offset = 0;
  for(int i = 0; i < blocks.size(); i++){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, offset, blocks[i].vertices.size() * sizeof(float), &blocks[i].vertices.front());
    offset += blocks[i].vertices.size() * sizeof(float);
  }
}

void Chunk::textureBlocks(){
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nrChannels;
  unsigned char *data = stbi_load("../img/dirt.jpeg", &width, &height, &nrChannels, 0);
  if (data)
  {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
      std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}

//This creates a bitmap we use to check adjacent blocks
void Chunk::generateVoxelGrid(){
  for(int x = 0; x < chunkSize; x++){
    for(int z = 0; z < chunkSize; z++){
      for(int y = 0; y < chunkSize; y++){
        voxelGrid[x][y][z] = 1;
      }
    }
  } 
}

//This checks to see what faces are visible
//Will eventually need to use the voxelGrid for this
bool Chunk::checkNeighbors(Block &b, int x, int y, int z) {
    bool faceDrawn = false;
    // Above
    if (y + 1 >= chunkSize) {
        b.insertVertices(topFace);
        faceDrawn = true;
    }

    // Below
    if (y - 1 < 0 ) {
        b.insertVertices(bottomFace);
        faceDrawn = true;
    }

    // Right
    if (x + 1 >= chunkSize ) {
        b.insertVertices(rightFace);
        faceDrawn = true;
    }

    // Left
    if (x - 1 < 0 ) {
        b.insertVertices(leftFace);
        faceDrawn = true;
    }

    // Front
    if (z - 1 < 0 ) {
        b.insertVertices(frontFace);
        faceDrawn = true;
    }

    // Behind
    if (z + 1 >= chunkSize ) {
        b.insertVertices(backFace);
        faceDrawn = true;
    }

    return faceDrawn;
}
void Chunk::drawChunk(){
  glDisable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, verticeCount);
  glBindVertexArray(0);
  glEnable(GL_CULL_FACE);
}
