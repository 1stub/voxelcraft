#include "raycast.h"
#include <glm/gtc/type_ptr.hpp>

Raycast::Raycast(Camera &cam, glm::mat4 proj) : camera(&cam), projectionMatrix(proj) {
  viewMatrix = camera->GetViewMatrix();
}

glm::vec3 Raycast::getCurrentRay(){
  return this->currentRay;
}

void Raycast::update(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    this->viewMatrix = camera->GetViewMatrix();
    this->projectionMatrix = camera->getProjMatrix();
    currentRay = calculateMouseRay();
}
glm::vec3 Raycast::calculateMouseRay(){
  glm::vec2 normalizedCoords = getNormalizedDeviceCoords();  
  glm::vec4 clipCoords = glm::vec4(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
  glm::vec4 eyeCoords = toEyeCoords(clipCoords); 
  glm::vec3 worldRay = toWorldCoords(eyeCoords);
  return worldRay;
}

glm::vec3 Raycast::toWorldCoords(glm::vec4 eyeCoords){
  glm::mat4 invertedView = glm::inverse(viewMatrix);
  glm::vec4 rayWorld = invertedView * eyeCoords;
  glm::vec3 mouseRay = glm::normalize(glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z));
  return rayWorld;
}

glm::vec4 Raycast::toEyeCoords(glm::vec4 clipCoords){
  glm::mat4 invertedProj = glm::inverse(projectionMatrix);
  glm::vec4 eyeCoords = invertedProj * clipCoords;
  return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
}

glm::vec3 Raycast::convertWorldToBlockCoords(const glm::vec3& worldCoords, const glm::vec3& blockSize) {
    return glm::floor(worldCoords / blockSize); // Convert world position to block coordinates
}

glm::vec2 Raycast::getNormalizedDeviceCoords() {
    float centerX = SCR_WIDTH / 2.0f;
    float centerY = SCR_HEIGHT / 2.0f;

    // our "mouse" is always center of screen:
    glm::vec2 coords;
    coords.x = (2.0f * centerX) / SCR_WIDTH - 1.0f;
    coords.y = -((2.0f * centerY) / SCR_HEIGHT - 1.0f);
    return coords;
}

