#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"

class chunkManager{
  public:
    chunkManager();
    void initChunks();
    void drawChunks();
  private:
    int renderDistance = 5;
    std::vector<Chunk> chunks;
};

#endif
