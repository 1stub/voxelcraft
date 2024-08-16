#ifndef RAYCAST_H
#define RAYCAST_H

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
//for now this will just print the world coord we are looking at and do nothing with it
//likely will just send this info over to the chunk manager once i determine what chunk we are in

//really these should exist globally to all files, not maintainable
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

//the goat - https://www.youtube.com/watch?v=DLKN0jExRIM
class Raycast{
  public:
    Raycast(Camera &cam, glm::mat4 proj);
    glm::vec3 getCurrentRay();
    void update(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    glm::vec3 calculateMouseRay();
    glm::vec3 toWorldCoords(glm::vec4 eyeCoords);
    glm::vec4 toEyeCoords(glm::vec4 clipCoords);
    glm::vec2 getNormalizedDeviceCoords(float mouseX, float mouseY);
    
  private:
    glm::vec3 currentRay;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    Camera camera;
};

#endif

