#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glut.h>

struct Block{
  float vertices[8][6]{ //vertices, colors
    -0.5, 0.5, 0.0,  1.0f, 0.0f, 0.0f, 
    -0.5, -0.5, 0.0,  0.0f, 1.0f, 0.0f,  
    0.5, 0.5, 0.0,  0.0f, 0.0f, 1.0f,  
    0.5, -0.5, 0.0,  0.0f, 1.0f, 0.0f, 

    //back face
    -0.5, 0.5, -1.0,  1.0f, 0.0f, 0.0f, 
    -0.5, -0.5, -1.0,  0.0f, 1.0f, 0.0f,  
    0.5, 0.5, -1.0,  0.0f, 0.0f, 1.0f,  
    0.5, -0.5, -1.0,  0.0f, 1.0f, 0.0f 
  };

  unsigned int indices[36]{
    0, 2, 3, 0, 3, 1,
    2, 6, 7, 2, 7, 3,
    6, 4, 5, 6, 5, 7,
    4, 0, 1, 4, 1, 5,
    0, 4, 6, 0, 6, 2,
    1, 5, 7, 1, 7, 3,
  };
};

class Chunk{
  public:
    Chunk();
    void initChunk();
    void drawChunk();
  private:
    unsigned int VBO, VAO, EBO;
    std::vector<Block> blocks;
    Block block;
    int chunkSize = 16;
    int chunkDepth = 64;
};
