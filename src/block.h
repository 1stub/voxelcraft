#ifndef BLOCK_H
#define BLOCK_H

#include <vector>

enum blockType{
  Grass = 0,
  GassTop = 1,
  Dirt = 2,
  Stone = 3,
};

class Block{
  public:
    Block(blockType b_type, float xPos, float yPos, float zPos) : type(b_type), x(xPos), y(yPos), z(zPos){

    }

    void insertVertices(std::vector<std::vector<float>> face, std::vector<std::vector<float>> tex){
        for (int i = 0; i < face.size(); i++) {
            std::vector<float> translatedVertex = face[i];
            translatedVertex[0] += x; // x position
            translatedVertex[1] += y; // y position
            translatedVertex[2] += z; // z position
            translatedVertex[3] = tex[i][0];
            translatedVertex[4] = tex[i][1];
            vertices.insert(vertices.end(), translatedVertex.begin(), translatedVertex.end());
        }
    }

    int getBlockId(){
      return type;
    }

    std::vector<float> vertices; 
  
  private:
    blockType type;
    float x;
    float y;
    float z;
};

#endif

