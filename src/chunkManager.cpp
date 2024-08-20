#include "chunkManager.h"

chunkManager::chunkManager(){
  for(int i = -renderDistance; i < renderDistance; i++){
    for(int j = -renderDistance; j < renderDistance; j++){
      Chunk c(i, j);
      chunks.push_back(c);
    }
  }
}

glm::vec3 chunkManager::checkRayAllChunks(Raycast &ray, Camera &camera){
  for(auto &c : chunks){
    glm::vec3 ret = c.checkRayIntersection(ray, camera);
    if( ret != glm::vec3(-1000, -1000, -1000)){
      return ret; 
    }
  }
  return glm::vec3(-1000, -1000, -1000);
}

void chunkManager::drawChunks(){
  for(auto &c : chunks){
    c.drawChunk();
  }
}
