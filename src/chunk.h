#ifndef CHUNK_H
#define CHUNK_H

#include <iostream>
#include <vector>
#include <map>

#include "camera.h"
#include "block.h"
#include "shader.h"
#include "raycast.h"
#include "perlin.hpp"

#include "../resources/stb_image.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glut.h>

typedef std::vector<std::vector<float>> blockTexCoords;

class Block;
class Chunk{
  friend Block;
  public:
    Chunk(int xOff, int zOff, const siv::PerlinNoise &perlin);
    int getNumBlocks();
    void initChunk();
    void updateVertices();
    void textureBlocks();
    void setBlockTexture();
    bool checkNeighbors(Block &b, int x, int y, int z);
    void generateHeightMap(const siv::PerlinNoise &p);
    glm::vec3 checkRayIntersection(Raycast &ray, Camera &c);
    double getNoiseValue(const siv::PerlinNoise &p, int x, int z);
    void drawChunk();
  private:
    unsigned int VBO, VAO;
    int chunkSize = 16;
    int chunkHeight = 256;
    int xOffset;
    int zOffset;
    unsigned char voxelGrid[16+2][256][16+2]; //+2 for padding
    std::vector<Block> blocks; 
    unsigned int texture;
    float verticeCount = 0;
    blockTexCoords blockTextures[4]; 
    
    //for siv::PerlinNoise
    int octaves = 8;
    float persistence = 0.5;
    float frequency = 0.01;

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
