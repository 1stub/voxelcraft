#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <string>

#include "globals.h"
#include "font.h"
#include "chunkManager.h" 
#include "raycast.h"

void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float deltaTime);

GLint pMode;

Camera camera;

int main(){
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
  GLFWwindow *window = glfwCreateWindow(Screen::width, Screen::height, "OpenGL Test", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSwapInterval(0); //enables vsync, 1 = locked at 60

  glfwSetCursorPosCallback(window, MouseCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  Shader shader("../shaders/shader.vs", "../shaders/shader.fs");
  Shader outlineShader("../shaders/outline.vs", "../shaders/outline.fs");

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glDepthFunc(GL_LESS);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //keeps mouse in game

  double prevTime = 0.0;
  double crntTime = 0.0;
  double timeDiff;
  double fpsUpdateTime = 0.0;
  unsigned int frameCount = 0;
  std::string FPS;

  Font font(Screen::width, Screen::height);
  chunkManager chunkManager;
  Raycast ray(camera, camera.getProjMatrix());

  unsigned int b_VBO, b_VAO; //these are used to render only verticies of selected block.
  glGenVertexArrays(1, &b_VAO);
  glGenBuffers(1, &b_VBO);
  
  //Our main game loop
while (!glfwWindowShouldClose(window)) {
    // Get current time and calculate the time difference
    crntTime = glfwGetTime();
    timeDiff = crntTime - prevTime;
    prevTime = crntTime;

    // Update frame count and FPS calculation
    frameCount++;
    fpsUpdateTime += timeDiff;

    if (fpsUpdateTime >= 1.0f) { // Update FPS every second
        FPS = std::to_string(frameCount); // Integer FPS
        frameCount = 0;
        fpsUpdateTime = 0.0f;
    }
    
    // Clear the screen
    glClearColor(0.4745f, 0.651f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update dt and process input
    processInput(window, static_cast<float>(timeDiff));
    

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); 

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (Screen::width / Screen::height), 0.1f, 100.0f);

    glm::ivec3 voxel = chunkManager.mouseVoxel(ray, camera);
    ray.update(camera.GetViewMatrix(), camera.getProjMatrix());
    glm::vec3 cameraWorldPos = camera.getCameraWorldPosition();
    
    // Rendering commands
    shader.use();

    int modelLoc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    int viewLoc = glGetUniformLocation(shader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    int projLoc = glGetUniformLocation(shader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
   
    glStencilFunc(GL_ALWAYS, 1, 0xFF); 
    glStencilMask(0xFF); 
    chunkManager.drawChunks();

    auto data = chunkManager.fetchBlockFromChunk(voxel);
    if(data.second > 0){
      outlineShader.use();
      //this means that we found a block and now need to send its information to our outline shader.      

      int outlineModelLoc = glGetUniformLocation(outlineShader.ID, "model");
      glUniformMatrix4fv(outlineModelLoc, 1, GL_FALSE, glm::value_ptr(model));

      int outlineViewLoc = glGetUniformLocation(outlineShader.ID, "view");
      glUniformMatrix4fv(outlineViewLoc, 1, GL_FALSE, glm::value_ptr(view));
      
      int outlineProjLoc = glGetUniformLocation(outlineShader.ID, "projection");
      glUniformMatrix4fv(outlineProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

      glBindVertexArray(b_VAO);  
      
      glBindBuffer(GL_ARRAY_BUFFER, b_VBO);
      glBufferData(GL_ARRAY_BUFFER, data.second * sizeof(float), data.first, GL_STATIC_DRAW);
      
      //position attrib
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
      
      glDisable(GL_CULL_FACE);
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00); 
      glDisable(GL_DEPTH_TEST);
      glDrawArrays(GL_TRIANGLES, 0, data.second);
      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);   
      glEnable(GL_DEPTH_TEST); 
      glEnable(GL_CULL_FACE);
      glBindVertexArray(0);
    }
    


    // Initialize the cooldown period and the last block break time
    static bool mouseButtonPressed = false;
    static auto lastBreakTime = std::chrono::high_resolution_clock::now();
    constexpr double cooldownDuration = 0.5; // Cooldown duration in seconds

    if (data.second > 0) {
        // Get the current time
        auto currentTime = std::chrono::high_resolution_clock::now();
        // Calculate the elapsed time since the last block break
        double elapsedTime = std::chrono::duration<double>(currentTime - lastBreakTime).count();

        // Check if the mouse button is pressed, the cooldown has passed, and the button is not already registered as pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouseButtonPressed && elapsedTime >= cooldownDuration) {
            // Break the block
            chunkManager.deleteBlock(voxel);
            // Set the flag to indicate the button is pressed
            mouseButtonPressed = true;
            // Update the last break time
            lastBreakTime = currentTime;
        }

        // Check if the mouse button is pressed, the cooldown has passed, and the button is not already registered as pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS && !mouseButtonPressed && elapsedTime >= cooldownDuration) {
            // Break the block
            chunkManager.placeBlock(voxel);
            // Set the flag to indicate the button is pressed
            mouseButtonPressed = true;
            // Update the last break time
            lastBreakTime = currentTime;
        }

        // Reset the flag when the button is released
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
            mouseButtonPressed = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
            mouseButtonPressed = false;
        }
    }

    //text rendering
    glDisable(GL_DEPTH_TEST);
    font.RenderText(FPS + " fps", 10.0f, Screen::height-20.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    font.RenderText("+", Screen::width/2.0f, Screen::height/2.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    font.RenderText(std::to_string(cameraWorldPos.x) + ", " + std::to_string(cameraWorldPos.y) + ", " + std::to_string(cameraWorldPos.z), 10.0f, Screen::height - 50.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    font.RenderText(std::to_string(ray.getCurrentRay().x) + ", " + std::to_string(ray.getCurrentRay().y) + ", " + std::to_string(ray.getCurrentRay().z), 10.0f, Screen::height - 80.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    font.RenderText(std::to_string(voxel.x) + ", " + std::to_string(voxel.y) + ", " + std::to_string(voxel.z), 10.0f, Screen::height - 110.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    glEnable(GL_DEPTH_TEST);


    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
}
  glfwTerminate();
  return 0;
}

float diff = 0.0f;
float prevT = 0.0f;

void processInput(GLFWwindow *window, float deltaTime){
    glGetIntegerv(GL_POLYGON_MODE, &pMode);
    
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    static bool fKeyPressed = false;
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed){
        fKeyPressed = true;
        if(pMode == GL_LINE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE){
        fKeyPressed = false;
    }
}

void MouseCallback(GLFWwindow *window, double xpos, double ypos) {
    static float lastX = Screen::width / 2.0f;
    static float lastY = Screen::height / 2.0f;
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
  glViewport(0,0, width, height);
}
