#include "raycast.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Raycast::Raycast(Camera &cam, glm::mat4 proj) : camera(cam), projectionMatrix(proj) {
  viewMatrix = camera.GetViewMatrix();
}

glm::vec3 Raycast::getCurrentRay(){
  return this->currentRay;
}

void Raycast::update(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    this->viewMatrix = viewMatrix;
    this->projectionMatrix = projectionMatrix;
    currentRay = calculateMouseRay();
}
glm::vec3 Raycast::calculateMouseRay(){
  float mouseX = SCR_WIDTH / 2.0f;
  float mouseY = SCR_HEIGHT / 2.0f;
  glm::vec2 normalizedCoords = getNormalizedDeviceCoords(mouseX, mouseY);  
  glm::vec4 clipCoords = glm::vec4(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
  glm::vec4 eyeCoords = toEyeCoords(clipCoords); 
  glm::vec3 worldRay = toWorldCoords(eyeCoords);
  return worldRay;
}

glm::vec3 Raycast::toWorldCoords(glm::vec4 eyeCoords){
  glm::mat4 invertedView = glm::inverse(viewMatrix);
  glm::vec4 rayWorld = invertedView * eyeCoords;
  glm::vec3 mouseRay = glm::normalize(glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z));
  return mouseRay;
}

glm::vec4 Raycast::toEyeCoords(glm::vec4 clipCoords){
  glm::mat4 invertedProj = glm::inverse(projectionMatrix);
  glm::vec4 eyeCoords = invertedProj * clipCoords;
  return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
}

glm::vec2 Raycast::getNormalizedDeviceCoords(float mouseX, float mouseY){
  glm::vec2 coords;
  coords.x = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
  coords.y = -((2.0f * mouseY) / SCR_HEIGHT - 1.0f);
  return coords;
}

