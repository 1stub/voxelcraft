#ifndef CHUNK_H
#define CHUNK_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <memory>

#include "globals.h"
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

struct Comp_ivec3 {
  size_t operator()(const glm::ivec3& vec) const {
    return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1) ^ (std::hash<int>()(vec.z) << 1);
  }
  bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  }
};

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
    std::vector<int> checkNeighbors(int x, int y, int z);
    void setFaces(glm::ivec3 bPos, std::vector<int> faces);
    void generateHeightMap(const siv::PerlinNoise &p);
    std::vector<glm::ivec3> getBlocks();
    glm::vec3 checkRayIntersection(Raycast &ray, Camera &c);
    double getNoiseValue(const siv::PerlinNoise &p, int x, int z);
    glm::vec3 mouseVoxel(Raycast &ray, Camera &camera);
    std::shared_ptr<Block> fetchBlock(glm::ivec3 blockCoords);
    void deleteBlock(glm::ivec3 voxel, const siv::PerlinNoise &p);
    void drawChunk();

    std::unordered_map<glm::ivec3, std::shared_ptr<Block>, Comp_ivec3, Comp_ivec3> blocks; 
  private:
    unsigned int VBO, VAO;
    int xOffset;
    int zOffset;
    unsigned char voxelGrid[Chunks::size+2][Chunks::height][Chunks::size+2]; //+2 for padding
    unsigned int texture;
    float verticeCount = 0;
    blockTexCoords blockTextures[4]; 
    
    //for siv::PerlinNoise
    int octaves = 8;
    float persistence = 0.5;
    float frequency = 0.01;

    //thinking it would be smarter to define texture coords seperate
    //currently goes first 3 pos last 2 tex
    std::vector<std::vector<float>> frontFace = {
        {1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    };

    std::vector<std::vector<float>> backFace = {
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    };

    std::vector<std::vector<float>> leftFace = {
        {0.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    };

    std::vector<std::vector<float>> rightFace = {
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    };

    std::vector<std::vector<float>> bottomFace = {
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    };

    std::vector<std::vector<float>> topFace = {
        {0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    };
};

#endif
