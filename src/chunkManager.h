#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "globals.h"
#include "chunk.h"
#include <unordered_set>
#include <unordered_map>
#include <memory>

struct BlockCoord {
  int x, y, z;

  bool operator==(const BlockCoord& other) const {
      return x == other.x && y == other.y && z == other.z;
  }
};

struct Comp_ivec2 {
  std::size_t operator()(const glm::ivec2& v) const {
      return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
  }
  bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
    return a.x == b.x && a.y == b.y;
  }
};


struct BlockCoordHash {
    std::size_t operator()(const BlockCoord& bc) const {
        std::size_t h1 = std::hash<int>()(bc.x);
        std::size_t h2 = std::hash<int>()(bc.y);
        std::size_t h3 = std::hash<int>()(bc.z);
        // Better mixing function
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

class chunkManager{
  public:
    chunkManager();
    void update(glm::vec3 playerPosition);
    void removeChunkFromActive(const glm::ivec2& chunkPos);
    void addChunk(const glm::ivec2& chunkPos, int i, int j, const glm::vec3& p);
    void initChunks();
    glm::vec3 checkRayAllChunks(Raycast &ray, Camera &camera);
    bool blockExists(int x, int y, int z) const;
    void deleteBlock(glm::ivec3 voxel);
    void placeBlock(glm::ivec3 voxel, blockType bType);
    std::pair<float*, int> fetchBlockFromChunk(glm::ivec3 blockCoords);
    std::unique_ptr<Chunk> fetchChunk(glm::ivec2 chunkCoords);
    std::pair<glm::vec3, glm::vec3> mouseVoxel(Raycast &ray, Camera &camera);
    void drawChunks();
  private:
    const siv::PerlinNoise p;
    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2> chunks;
    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2> chunkCache;
    std::unordered_set<BlockCoord, BlockCoordHash> blockManager; //just to check if a block exists
};

#endif
