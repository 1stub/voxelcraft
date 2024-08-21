#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"
#include <unordered_set>

struct BlockCoord {
  int x, y, z;

  bool operator==(const BlockCoord& other) const {
      return x == other.x && y == other.y && z == other.z;
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
    glm::vec3 mouseVoxel(Raycast &ray, Camera &camera);
    void drawChunks();
  private:
    int renderDistance = 5;
    const siv::PerlinNoise p;
    std::vector<Chunk> chunks;
    std::unordered_set<BlockCoord> blockManager;
};

#endif
