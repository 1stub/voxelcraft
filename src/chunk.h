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

class Block;
class Chunk{
  friend Block;
  public:
    Chunk();
    void initChunk();
    void updateVertices();
    void drawChunk();
    void textureBlocks();
  private:
    unsigned int VBO, VAO;
    std::vector<Block> blocks;
    
    unsigned int texture;
    int chunkSize = 16;
};

#endif
