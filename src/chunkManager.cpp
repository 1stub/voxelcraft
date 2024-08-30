#include "chunkManager.h"
#include <float.h>

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
    auto blockIt = chunks[chunkCoords]->blocks.find(blockCoords);

    if (blockIt != chunks[chunkCoords]->blocks.end() && blockIt->second) {
        return {blockIt->second->vertices.data(), blockIt->second->vertices.size()};

        std::cout << "Block found at coordinates: " << blockCoords.x << ", " << blockCoords.y << ", " << blockCoords.z << std::endl;
    } else {
        std::vector<float> ret;
        std::cerr << "Chunk not found at coordinates: " << chunkCoords.x << ", " << chunkCoords.y << std::endl;
        std::cerr << "Block not found at coordinates: " << blockCoords.x << ", " << blockCoords.y << ", " << blockCoords.z << std::endl;
        return {ret.data(), 0};// return an empty vector or handle the error as needed
    }
}

//https://github.com/rhysboer/VoxitCraft/blob/master/Minecraft/Raycast.cpp
//code slightly modified from above ^, could not figure out implemntation on my own
glm::vec3 chunkManager::mouseVoxel(Raycast &ray, Camera &camera) {
    using namespace glm;

    float distance = 10.0f;
    float cellSize = 1.0f;

    vec3 rayWOR = ray.getCurrentRay(); //normalized direction vector from ray (at enter of screen)
    vec3 startPoint = camera.getCameraWorldPosition();

    vec3 endPoint = startPoint + (rayWOR * distance);

    vec3 startCell (startPoint.x < 0.0f ? ceil(startPoint.x) : floor(startPoint.x), startPoint.y < 0.0f ? ceil(startPoint.y) : floor(startPoint.y) , startPoint.z < 0.0f ? ceil(startPoint.z) : floor(startPoint.z)); /*floor(startPoint)*/
    vec3 endCell ( (endPoint.x < 0.0f) ? ceil(endPoint.x) : floor(endPoint.x), (endPoint.y < 0.0f) ? ceil(endPoint.y) : floor(endPoint.y) , (endPoint.z < 0.0f) ? ceil(endPoint.z) : floor(endPoint.z)); /*floor(endPoint)*/

    vec3 direction = endPoint - startPoint;
    vec3 norm_direction = normalize(direction);

    // Determine the step direction for raycasting
    int stepX = sign(rayWOR.x);
    int stepY = sign(rayWOR.y);
    int stepZ = sign(rayWOR.z);

    // Determine the step direction for raycasting
    // Distance to nearest square side
    float near_x = (stepX >= 0) ? ((startCell.x + 1) * cellSize - startPoint.x) : (startPoint.x - (startCell.x * cellSize)) ;
    float near_y = (stepY >= 0) ? ((startCell.y + 1) * cellSize - startPoint.y) : (startPoint.y - (startCell.y * cellSize)) ;
    float near_z = (stepZ >= 0) ? ((startCell.z + 1) * cellSize - startPoint.z) : (startPoint.z - (startCell.z * cellSize)) ;

    const float EPSILON = 1e-6f;
    float maxX = (std::abs(norm_direction.x) > EPSILON) ? near_x / norm_direction.x : DBL_MAX;
    float maxY = (std::abs(norm_direction.y) > EPSILON) ? near_y / norm_direction.y : DBL_MAX;
    float masZ = (std::abs(norm_direction.z) > EPSILON) ? near_z / norm_direction.z : DBL_MAX;

    float dx = (std::abs(norm_direction.x) > EPSILON) ? cellSize / norm_direction.x : DBL_MAX;
    float dy = (std::abs(norm_direction.y) > EPSILON) ? cellSize / norm_direction.y : DBL_MAX;
    float dz = (std::abs(norm_direction.z) > EPSILON) ? cellSize / norm_direction.z : DBL_MAX;

    vec3 pos = startPoint; 

    int gridBoundX = std::abs(endCell.x - startCell.x);
    int gridBoundY = std::abs(endCell.y - startCell.y);
    int gridBoundZ = std::abs(endCell.z - startCell.z);

    int counter = 0;

    while(counter != (gridBoundX + gridBoundY + gridBoundZ)) {
      if(std::abs(maxX) < std::abs(maxY)) {
        if(std::abs(maxX) < std::abs(masZ)) {
          maxX += dx;
          pos.x += stepX;
        } else {
          masZ += dz;
          pos.z += stepZ;
        }
      } else {
        if(std::abs(maxY) < std::abs(masZ)) {
          maxY += dy;
          pos.y += stepY;
        } else {
          masZ += dz;
          pos.z += stepZ;
        }
      }
 
      if(pos.y < 0)
        break;

      if(blockExists(pos.x, pos.y, pos.z)) {
        return pos;
      }
      ++counter;
    }

    return vec3(0.0, -1.0, 0.0);
}

void chunkManager::drawChunks(){
  for(auto &c : chunks){
    c.second->drawChunk();
  }
}
