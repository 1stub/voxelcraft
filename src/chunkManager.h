#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

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
  size_t operator()(const glm::ivec2& vec) const {
    return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
  }
  bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
    return a.x == b.x && a.y == b.y;
  }
};

//is this efficient? maybe? does it work? yes
namespace std {
    template <>
    struct hash<BlockCoord> {
        size_t operator()(const BlockCoord& coord) const {
            size_t h1 = std::hash<int>{}(coord.x);
            size_t h2 = std::hash<int>{}(coord.y);
            size_t h3 = std::hash<int>{}(coord.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); // Combine hash values
        }
    };
}

class chunkManager{
  public:
    chunkManager();
    void initChunks();
    glm::vec3 checkRayAllChunks(Raycast &ray, Camera &camera);
    bool blockExists(int x, int y, int z) const;
    std::vector<float> fetchBlockFromChunk(glm::ivec3 blockCoords);
    Chunk fetchChunk(glm::ivec2 chunkCoords);
    glm::vec3 mouseVoxel(Raycast &ray, Camera &camera);
    void drawChunks();
  private:
    int renderDistance = 5;
    const siv::PerlinNoise p;
    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, Comp_ivec2, Comp_ivec2> chunks;
    std::unordered_set<BlockCoord> blockManager; //just to check if a block exists
};

#endif
