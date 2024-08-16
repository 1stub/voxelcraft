#include "chunkManager.h"

chunkManager::chunkManager(){
  for(int i = -renderDistance; i < renderDistance; i++){
    for(int j = -renderDistance; j < renderDistance; j++){
      Chunk c(i, j);
      chunks.push_back(c);
    }
  }
}

void chunkManager::drawChunks(){
  for(auto &c : chunks){
    c.drawChunk();
  }
}
