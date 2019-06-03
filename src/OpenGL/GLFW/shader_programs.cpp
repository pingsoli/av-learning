// using two shader program to render two triangles
// one is orange, and another is yello color.
// two shader programs using sperate vertex shader and fragment shader.

#include "utils.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const char* vertexShaderSource =
    "layout (location = 0) in vec3 aPos;"
    "void main()"
    "{"
    "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
    "}";

const char *fragmentShaderOrangeSource =
    "out vec4 FragColor;"
    "void main()"
    "{"
    "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
    "}";

const char* fragmentShaderYellowSource =
    "out vec4 FragColor;"
    "void main()"
    "{"
    "  FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);"
    "}";

int main(int argc, char* argv[])
{
  const unsigned int width = 800;
  const unsigned int height = 600;

  GLFWwindow *window = createWindow(width, height, "OpenGL Excercises", framebuffer_size_callback);
  // Build and compile our shader program
  // vertex shader
  uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  // fragment shader
  uint32_t orangeFragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderOrangeSource);
  uint32_t yellowFragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderYellowSource);

  int orangeShaderProgram = glCreateProgram();
  glAttachShader(orangeShaderProgram, vertexShader);
  glAttachShader(orangeShaderProgram, orangeFragmentShader);
  glLinkProgram(orangeShaderProgram);

  int yellowShaderProgram = glCreateProgram();
  glAttachShader(yellowShaderProgram, vertexShader);
  glAttachShader(yellowShaderProgram, yellowFragmentShader);
  glLinkProgram(yellowShaderProgram);

  float firstTriangle[] = {
    -0.5f, 0.5f, 0.0f,
    -0.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f,
  };
  float secondTriangle[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f,
    0.5f, 0.0f, 0.0f
  };

  uint32_t VBO[2], VAO[2];
  glGenBuffers(2, VBO);
  glGenVertexArrays(2, VAO);

  glBindVertexArray(VAO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(VAO[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(secondTriangle), secondTriangle, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // render  window
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(orangeShaderProgram);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(yellowShaderProgram);
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(2, VBO);
  glDeleteVertexArrays(2, VAO);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}