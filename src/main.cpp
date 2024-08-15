//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include <string>

#include "font.h"
#include "chunk.h" 

//  g++ -o prog main.cpp glad.c -lGL -lglfw

void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float deltaTime);

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

GLint pMode;

Camera camera;

int main(){
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Test", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSwapInterval(1); //enables vsync, 0 = locked at 60

  glfwSetCursorPosCallback(window, MouseCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  Shader shader("../shaders/shader.vs", "../shaders/shader.fs");

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //keeps mouse in game


  double prevTime = 0.0;
  double crntTime = 0.0;
  double timeDiff;
  unsigned int counter = 0;
  std::string FPS;

  Font font(SCR_WIDTH, SCR_HEIGHT);
  Chunk chunk;

  //Our main game loop
  while(!glfwWindowShouldClose(window)){
    crntTime = glfwGetTime();
    timeDiff = crntTime - prevTime;
    counter++;
    if(timeDiff >= 1.0 / 30.0){
      FPS = std::to_string((1.0 / timeDiff) * counter);
    }

    processInput(window, static_cast<float>(crntTime));

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    //rendering commands here
    shader.use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.5f, 1.0f, 0.0f)); 

    glm::mat4 view;
    view = camera.GetViewMatrix();

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);

    int modelLoc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    int viewLoc = glGetUniformLocation(shader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    int projLoc = glGetUniformLocation(shader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
 
    chunk.drawChunk();
  
    glDisable(GL_DEPTH_TEST);
    font.RenderText(FPS,1540.0f, 870.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
    font.RenderText(std::to_string(chunk.getNumBlocks()), 1200.0f, 870.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
    glEnable(GL_DEPTH_TEST);
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
  if(glfwGetKey(window, GLFW_KEY_F)){
    diff = deltaTime - prevT;
    if(std::abs(diff) > 0.5){
      prevT = deltaTime;
      if(pMode == GL_LINE){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }
    }
  }
}

void MouseCallback(GLFWwindow *window, double xpos, double ypos) {
    static float lastX = SCR_WIDTH / 2.0f;
    static float lastY = SCR_HEIGHT / 2.0f;
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
  glViewport(0,0, width, height);
}



