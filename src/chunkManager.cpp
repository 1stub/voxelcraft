#include "chunkManager.h"

chunkManager::chunkManager(){
  for(int i = -renderDistance; i < renderDistance; i++){
    for(int j = -renderDistance; j < renderDistance; j++){
      chunks.emplace(glm::ivec2(i,j), std::make_unique<Chunk>(i, j, p));
      for(auto &b : chunks[glm::ivec2(i,j)]->getBlocks()){
        blockManager.insert({b.x, b.y, b.z});
      }
    }
  }
}

bool chunkManager::blockExists(int x, int y, int z) const {
    BlockCoord coord = {x, y, z};
    return blockManager.find(coord) != blockManager.end();
}

Block chunkManager::fetchBlockFromChunk(glm::ivec3 blockCoords){
  glm::ivec2 chunkCoords(blockCoords.x/16, blockCoords.y / 16); //hard coded for now, 16 is our chunk size
  Chunk c = fetchChunk(chunkCoords);
  //Block b = c.fetchBlock(blockCoords);
}

Chunk chunkManager::fetchChunk(glm::ivec2 chunkCoords){
  return *chunks[glm::ivec2(chunkCoords.x, chunkCoords.y)]; 
}

glm::vec3 chunkManager::mouseVoxel(Raycast &ray, Camera &camera) {
    using namespace glm;
    
    // Retrieve the ray in world space
    vec3 rayWOR = ray.getCurrentRay();
    
    // Camera position
    vec3 camPos = camera.getCameraWorldPosition();
    
    // Voxel grid position
    int xPos = static_cast<int>(floor(camPos.x));
    int yPos = static_cast<int>(floor(camPos.y));
    int zPos = static_cast<int>(floor(camPos.z));
    
    // Determine the step direction for raycasting
    int stepX = sign(rayWOR.x);
    int stepY = sign(rayWOR.y);
    int stepZ = sign(rayWOR.z);
    
    // Compute tMax and tDelta for ray stepping
    vec3 tMax(
        (stepX > 0 ? (xPos + 1 - camPos.x) : (camPos.x - xPos)) / rayWOR.x,
        (stepY > 0 ? (yPos + 1 - camPos.y) : (camPos.y - yPos)) / rayWOR.y,
        (stepZ > 0 ? (zPos + 1 - camPos.z) : (camPos.z - zPos)) / rayWOR.z
    );
    
    vec3 tDelta(
        std::abs(1.0f / rayWOR.x),
        std::abs(1.0f / rayWOR.y),
        std::abs(1.0f / rayWOR.z)
    );
    
    // Variables to determine which face of the voxel is hit
    float faceX = 0, faceY = 0, faceZ = 0;
    
    // Main raycasting loop
    float range = 1000.0f; // Max range to check
    while (tMax.x < range && tMax.y < range && tMax.z < range) {
      //need to see if a block exists at these coordinates
        if (blockExists(xPos, yPos, zPos)) {
            // Return the coordinates of the intersected voxel
            return vec3(xPos, yPos, zPos);
        }
        
        // Determine the voxel face to step into
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                if (tMax.x > range) break;
                xPos += stepX;
                tMax.x += tDelta.x;
                faceX = -stepX;
                faceY = 0;
                faceZ = 0;
            } else {
                if (tMax.z > range) break;
                zPos += stepZ;
                tMax.z += tDelta.z;
                faceX = 0;
                faceY = 0;
                faceZ = -stepZ;
            }
        } else {
            if (tMax.y < tMax.z) {
                if (tMax.y > range) break;
                yPos += stepY;
                tMax.y += tDelta.y;
                faceX = 0;
                faceY = -stepY;
                faceZ = 0;
            } else {
                if (tMax.z > range) break;
                zPos += stepZ;
                tMax.z += tDelta.z;
                faceX = 0;
                faceY = 0;
                faceZ = -stepZ;
            }
        }
    }
    
    // Return a vec3 indicating no intersection (optional, could be a sentinel value)
    return vec3(-1.0f, -1.0f, -1.0f);
}

void chunkManager::drawChunks(){
  for(auto &c : chunks){
    c.second->drawChunk();
  }
}
