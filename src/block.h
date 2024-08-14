#ifndef BLOCK_H
#define BLOCK_H

#include <vector>

class Block{
  public:
    Block(float xPos, float yPos, float zPos) : x(xPos), y(yPos), z(zPos){

    }

    void insertVertices(std::vector<std::vector<float>> face){
        for (const auto& vertex : face) {
            std::vector<float> translatedVertex = vertex;
            translatedVertex[0] += x; // x position
            translatedVertex[1] += y; // y position
            translatedVertex[2] += z; // z position
            vertices.insert(vertices.end(), translatedVertex.begin(), translatedVertex.end());
        }
    }
    std::vector<float> vertices; 
  
  private:
    float x;
    float y;
    float z;
};

#endif

