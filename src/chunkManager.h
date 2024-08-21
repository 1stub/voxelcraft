#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"

class chunkManager{
  public:
    chunkManager();
    void initChunks();
    glm::vec3 checkRayAllChunks(Raycast &ray, Camera &camera);
    void drawChunks();
  private:
    int renderDistance = 1;
    const siv::PerlinNoise p;
    std::vector<Chunk> chunks;
};

#endif
