#include "chunkManager.h"
#include "globals.h"

chunkManager::chunkManager(){
  for(int i = -Render::renderDistance; i < Render::renderDistance; i++){
    for(int j = -Render::renderDistance; j < Render::renderDistance; j++){
      glm::ivec2 chunkPos(i,j);
      chunks.emplace(chunkPos, std::make_unique<Chunk>(i, j, p));
      for(auto &b : chunks[chunkPos]->getBlocks()){
        blockManager.insert({b.x, b.y, b.z});
      }
    }
  }
  drawChunks();
}
void chunkManager::update(glm::vec3 playerPosition){
  glm::ivec2 chunkCoords(
    playerPosition.x >= 0 ? playerPosition.x / Chunks::size : (playerPosition.x - Chunks::size + 1) / Chunks::size,
    playerPosition.z >= 0 ? playerPosition.z / Chunks::size : (playerPosition.z - Chunks::size + 1) / Chunks::size
  );
  std::unordered_set<glm::ivec2, Comp_ivec2> activeChunks;
  for(int i = chunkCoords.x - Render::renderDistance; i < chunkCoords.x + Render::renderDistance; i++){
    for(int j = chunkCoords.y - Render::renderDistance; j < chunkCoords.y + Render::renderDistance; j++){
        glm::ivec2 chunkPos(i,j);
        activeChunks.insert(chunkPos);
        if(chunkCache.find(chunkPos) != chunkCache.end()){
          auto it = chunkCache.find(chunkPos); 
          if(it != chunkCache.end()){
            chunks[it->first] = std::move(it->second);
            chunkCache.erase(it);
          }
        }else{
        if(chunks.find(chunkPos) == chunks.end()){
          chunks.emplace(chunkPos, std::make_unique<Chunk>(i, j, p));
          for(auto &b : chunks[chunkPos]->getBlocks()){
            blockManager.insert({b.x, b.y, b.z});
          }
        }
      }
    }
  }

  std::vector<glm::ivec2> chunksToMove; //cant delete iterator while iterating lol
  for(auto &c : chunks){
    if(activeChunks.find(c.first) == activeChunks.end()){ //chunk out of render distance, needs to be cached
      chunksToMove.push_back(c.first);
    }
  }
  for(const auto &chunkPos : chunksToMove){
      auto it = chunks.find(chunkPos);
      if(it != chunks.end() && chunkCache.find(chunkPos) == chunkCache.end()){
        chunkCache[chunkPos] = std::move(it->second);
        chunks.erase(it);
      }
  }

  std::cout << chunkCache.size() << std::endl;
}

void chunkManager::removeChunkFromActive(const glm::ivec2& chunkPos){
  
}
    
void chunkManager::addChunk(const glm::ivec2& chunkPos, int i, int j, const glm::vec3& p){

}

void chunkManager::deleteBlock(glm::ivec3 voxel){
  BlockCoord blockCoords = {voxel.x, voxel.y, voxel.z};
  glm::ivec2 chunkCoords(
    blockCoords.x >= 0 ? blockCoords.x / Chunks::size : (blockCoords.x - Chunks::size + 1) / Chunks::size,
    blockCoords.z >= 0 ? blockCoords.z / Chunks::size : (blockCoords.z - Chunks::size + 1) / Chunks::size
  );
  blockManager.erase(blockCoords);

  //since worlds are (supposed) to be infinite we can assume these always exist.
  std::vector<Chunk*> adjChunks;
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x + 1, chunkCoords.y)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x - 1, chunkCoords.y)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x, chunkCoords.y + 1)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x, chunkCoords.y - 1)].get());

  chunks[chunkCoords]->deleteBlock(voxel, p, adjChunks);

  //NOTE !!!!!!
  //This needs to be changed - only for testing purposes now
  //there is no need to clear the whole manager when only one chunks blocks change. BlockManager needs reworked 
  //for the sake of updating blocks on both placement and deletion
  for (auto &b : chunks[chunkCoords]->getBlocks()) {
    blockManager.insert({b.x, b.y, b.z});
  }
}

void chunkManager::placeBlock(glm::ivec3 voxel, blockType bType){
  BlockCoord blockCoords = {voxel.x, voxel.y, voxel.z};
  glm::ivec2 chunkCoords(
    blockCoords.x >= 0 ? blockCoords.x / Chunks::size : (blockCoords.x - Chunks::size + 1) / Chunks::size,
    blockCoords.z >= 0 ? blockCoords.z / Chunks::size : (blockCoords.z - Chunks::size + 1) / Chunks::size
  );
  std::vector<Chunk*> adjChunks;
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x + 1, chunkCoords.y)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x - 1, chunkCoords.y)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x, chunkCoords.y + 1)].get());
  adjChunks.push_back(chunks[glm::ivec2(chunkCoords.x, chunkCoords.y - 1)].get());

  blockManager.insert(blockCoords);
  chunks[chunkCoords]->placeBlock(voxel, bType, adjChunks);
}


bool chunkManager::blockExists(int x, int y, int z) const {
    BlockCoord coord = {x, y, z};
    return blockManager.find(coord) != blockManager.end();
}

std::pair<float*, int> chunkManager::fetchBlockFromChunk(glm::ivec3 blockCoords){      
    glm::ivec2 chunkCoords(
        blockCoords.x >= 0 ? blockCoords.x / Chunks::size : (blockCoords.x - Chunks::size + 1) / Chunks::size,
        blockCoords.z >= 0 ? blockCoords.z / Chunks::size : (blockCoords.z - Chunks::size + 1) / Chunks::size
    );

    // Find the chunk
    auto chunkIt = chunks.find(chunkCoords);
    if (chunkIt != chunks.end()) {
        // Access the chunk and block
        auto& chunk = chunkIt->second;
        auto blockIt = chunk->blocks.find(blockCoords);
        
        if (blockIt != chunk->blocks.end() && blockIt->second) {
            // Return block vertices and size
            return {blockIt->second->vertices.data(), static_cast<int>(blockIt->second->vertices.size())};
        }
    }

    // Return an empty result if not found
    return {nullptr, 0};
}

//https://github.com/rhysboer/VoxitCraft/blob/master/Minecraft/Raycast.cpp
//code slightly modified from above ^, could not figure out implemntation on my own

// Helper function to determine the sign of a number
template <typename T>
int signum(T val) {
    return (T(0) < val) - (val < T(0));
}

// Helper function to compute the initial intersection point along a ray
float intbound(float s, float ds) {
    // ds should never be 0, as it's the direction vector component
    if (ds == 0.0f) return std::numeric_limits<float>::infinity();
    return (ds > 0.0f) ? (1.0f - s + std::floor(s)) / ds : (s - std::floor(s)) / -ds;
}

//https://gamedev.stackexchange.com/questions/72120/how-do-i-find-voxels-along-a-ray?noredirect=1&lq=1
//based off this ^ - note, i wasnt able to sucessful modify this code but chat gpt was :)
std::pair<glm::vec3, glm::vec3> chunkManager::mouseVoxel(Raycast &ray, Camera &camera) { //actual block, adjacent based on face normal
  glm::vec3 rayWOR = ray.getCurrentRay();

  // Set the starting position and steps for the ray
  float range = 8.0f; // max range to check (in voxels)
  glm::vec3 camPos = camera.getCameraWorldPosition();
  int xPos = std::floor(camPos.x);
  int yPos = std::floor(camPos.y);
  int zPos = std::floor(camPos.z);
  int stepX = signum(rayWOR.x);
  int stepY = signum(rayWOR.y);
  int stepZ = signum(rayWOR.z);

  // Compute initial tMax and tDelta values for DDA
  glm::vec3 tMax = glm::vec3(intbound(camPos.x, rayWOR.x), intbound(camPos.y, rayWOR.y), intbound(camPos.z, rayWOR.z));
  glm::vec3 tDelta = glm::vec3((float)stepX / rayWOR.x, (float)stepY / rayWOR.y, (float)stepZ / rayWOR.z);
  glm::vec3 faceNormal;

  // Ray traversal loop
  do {
      // Check if current voxel is solid
      if (blockExists(xPos, yPos, zPos)) {
          std::cout << "Intersected face normal: (" << faceNormal.x << ", " << faceNormal.y << ", " << faceNormal.z << ")" << std::endl;
          std::cout << "Solid voxel hit at: (" << xPos << ", " << yPos << ", " << zPos << ")" << std::endl;
          glm::vec3 voxel(xPos, yPos, zPos);
          return std::make_pair(voxel, voxel + faceNormal);
      }

      // Move to the next voxel based on tMax
      if (tMax.x < tMax.y) {
          if (tMax.x < tMax.z) {
              if (tMax.x > range) break;
              xPos += stepX;
              tMax.x += tDelta.x;
              faceNormal = glm::vec3(-stepX, 0, 0);
          } else {
              if (tMax.z > range) break;
              zPos += stepZ;
              tMax.z += tDelta.z;
              faceNormal = glm::vec3(0, 0, -stepZ);
          }
      } else {
          if (tMax.y < tMax.z) {
              if (tMax.y > range) break;
              yPos += stepY;
              tMax.y += tDelta.y;
              faceNormal = glm::vec3(0, -stepY, 0);
          } else {
              if (tMax.z > range) break;
              zPos += stepZ;
              tMax.z += tDelta.z;
              faceNormal = glm::vec3(0, 0, -stepZ);
          }
      }
  } while (true);
  return std::make_pair(glm::vec3(0,0,0), glm::vec3(0,0,0)); 
}

void chunkManager::drawChunks(){
  for(auto &c : chunks){
    c.second->drawChunk();
  }
}
