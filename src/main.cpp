#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <thread>
#include <mutex>

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
  
  static blockType blockTypeToPlace = Stone;
  static std::string blockType = "Stone";
  std::mutex updateMutex;
  //Our main game loop
while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClearColor(0.4745f, 0.651f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Get current time and calculate the time difference

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); 

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (Screen::width / Screen::height), 0.1f, 100.0f);

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

    chunkManager.update(camera.getCameraWorldPosition());
    //
    chunkManager.drawChunks();    
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

    // Update dt and process input
    processInput(window, static_cast<float>(timeDiff));

    auto voxel = chunkManager.mouseVoxel(ray, camera);
    ray.update(camera.GetViewMatrix(), camera.getProjMatrix());
    glm::vec3 cameraWorldPos = camera.getCameraWorldPosition();

    // Initialize the cooldown period and the last block break time
    static bool mouseButtonPressed = false;
    static auto lastBreakTime = std::chrono::high_resolution_clock::now();
    constexpr double cooldownDuration = 0.25; // Cooldown duration in seconds

    auto data = chunkManager.fetchBlockFromChunk(voxel.first);

    if (data.second) {
        // Get the current time
        auto currentTime = std::chrono::high_resolution_clock::now();
        // Calculate the elapsed time since the last block break
        double elapsedTime = std::chrono::duration<double>(currentTime - lastBreakTime).count();
      
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouseButtonPressed && elapsedTime >= cooldownDuration) {
            chunkManager.deleteBlock(voxel.first);
            mouseButtonPressed = true;
            lastBreakTime = currentTime;
        }
        
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
          blockTypeToPlace = Stone;
          blockType = "Stone";
        }
        else if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
          blockTypeToPlace = Dirt;
          blockType = "Dirt";
        }
        else if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
          blockTypeToPlace = WoodenPlank;
          blockType = "Wooden Plank";
        }
        else if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS){
          blockTypeToPlace = Leaf;
          blockType = "Leaf";
        }

        font.RenderText(blockType, Screen::width / 2.0f, Screen::height-(Screen::height-20.0f), 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS && !mouseButtonPressed && elapsedTime >= cooldownDuration) {
            chunkManager.placeBlock(voxel.second, blockTypeToPlace);
            mouseButtonPressed = true;
            lastBreakTime = currentTime;
        }

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
    font.RenderText(std::to_string(voxel.first.x) + ", " + std::to_string(voxel.first.y) + ", " + std::to_string(voxel.first.z), 10.0f, Screen::height - 110.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
    glEnable(GL_DEPTH_TEST);

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
      
      // Depth mask and cull face adjustments
      glDisable(GL_CULL_FACE);  // Disable culling to ensure transparency renders correctly
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // Only render where the stencil is not equal
      glStencilMask(0x00);      // Don't write to the stencil buffer
      glDisable(GL_DEPTH_TEST); // Disable depth test when drawing the transparent object
      glDepthMask(GL_FALSE);    // Prevent depth writes
      glDrawArrays(GL_TRIANGLES, 0, data.second); // Draw highlighted block
      glDepthMask(GL_TRUE);     // Re-enable depth writes
      glStencilMask(0xFF);      // Reset stencil mask
      glStencilFunc(GL_ALWAYS, 1, 0xFF); // Reset stencil function
      glEnable(GL_DEPTH_TEST);  // Re-enable depth testing
      glEnable(GL_CULL_FACE);   // Re-enable face culling
      glBindVertexArray(0);
    }


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
