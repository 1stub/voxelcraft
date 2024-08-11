#include "chunk.h"

Chunk::Chunk(){
  initChunk();
  //set blocks
  //for(int x = 0; x < chunkSize; x++){
    //for(int y = 0; y < chunkSize; y++){
      Block b(1.0f, 1.0f, 0.0f);
      updateVertices(b);
      //blocks.push_back(b);
    //}
  //}
}

void Chunk::initChunk(){
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);  
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 5, nullptr, GL_DYNAMIC_DRAW);
  
  //position attrib
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // texture attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float))); //we have to offset by 3*sizeof(float);
  glEnableVertexAttribArray(1);
}

//we need to update our vbo with new vertex data
void Chunk::updateVertices(Block &b){
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(b.vertices), b.vertices);
  textureBlocks();
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

void Chunk::drawChunk(){
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}
