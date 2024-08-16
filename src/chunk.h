#ifndef CHUNK_H
#define CHUNK_H

#include <iostream>
#include <vector>

#include "camera.h"
#include "block.h"
#include "shader.h"

#include "../resources/stb_image.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glut.h>

typedef std::vector<std::vector<float>> blockTexCoords;

class Block;
class Chunk{
  friend Block;
  public:
    Chunk(int xOffset, int zOffset);
    int getNumBlocks();
    void initChunk();
    void updateVertices();
    void textureBlocks();
    void setBlockTexture();
    bool checkNeighbors(Block &b, int x, int y, int z);
    void generateVoxelGrid();
    void drawChunk();
  private:
    unsigned int VBO, VAO;
    int chunkSize = 16;
    int voxelGrid[16][16][16];
    std::vector<Block> blocks; 
    unsigned int texture;
    float verticeCount = 0;
    blockTexCoords blockTextures[4];  
    
  std::vector<std::vector<float>> frontFace = {
      { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f},
      {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f},
      { 0.5f, -0.5f, -0.5f, 1.0f, 0.0f},
      { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f},
  };

  std::vector<std::vector<float>> backFace = {
      { 0.5f,  0.5f,  0.5f, 1.0f, 1.0f},
      {-0.5f,  0.5f,  0.5f, 0.0f, 1.0f},
      {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f},
      {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f},
      { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f},
      { 0.5f,  0.5f,  0.5f, 1.0f, 1.0f}
  };

  std::vector<std::vector<float>> leftFace = {
      {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f},
      {-0.5f,  0.5f, -0.5f, 1.0f, 1.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
      {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f},
      {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f}
  };

  std::vector<std::vector<float>> rightFace = {
      { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f},
      { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f},
      { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
      { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
      { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f},
      { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f}
  };

  std::vector<std::vector<float>> bottomFace = {
      {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
      { 0.5f, -0.5f, -0.5f, 1.0f, 1.0f},
      { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f},
      { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f},
      {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f},
      {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f}
  };

  std::vector<std::vector<float>> topFace = {
      {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f},
      { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f},
      { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f},
      { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f},
      {-0.5f,  0.5f,  0.5f, 0.0f, 0.0f},
      {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f}
  };
};

#endif
