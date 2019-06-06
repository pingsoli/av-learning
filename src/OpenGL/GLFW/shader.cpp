#include "utils.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// const char *vertexShaderSource =
//     "layout (location = 0) in vec3 aPos;"
//     "out vec4 vertexColor;"
//     "void main()"
//     "{"
//     "  gl_Position = vec4(aPos, 1.0);"
//     "  vertexColor = vec4(0.5, 0.0, 0.0, 1.0);"
//     "}";

// const char *fragmentShaderSource =
//     "out vec4 FragColor;"
//     "uniform vec4 ourColor;"
//     "void main()"
//     "{"
//     "  FragColor = ourColor;"
//     "}";

const char *vertexShaderSource =
    "layout (location = 0) in vec3 aPos;"
    "layout (location = 1) in vec3 aColor;"
    "out vec3 ourColor;"
    "void main()"
    "{"
    "  gl_Position = vec4(aPos, 1.0);"
    "  ourColor = aColor;"
    "}";

// // upside down the triangle
// const char *vertexShaderSource =
//     "layout (location = 0) in vec3 aPos;"
//     "layout (location = 1) in vec3 aColor;"
//     "out vec3 ourColor;"
//     "void main()"
//     "{"
//     "  gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);"
//     "  ourColor = aColor;"
//     "}";

const char *fragmentShaderSource =
    "out vec4 FragColor;"
    "in vec3 ourColor;"
    "void main()"
    "{"
    "  FragColor = vec4(ourColor, 1.0);"
    "}";

int main(int argc, char* argv[])
{
  const int width = 800;
  const int height = 600;
  GLFWwindow* window = createWindow(width, height, "shader learning", framebuffer_size_callback);

  uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  uint32_t shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // position only
  // float vertices[] = {
  //   -0.5f, -0.5f, 0.0f, // left
  //    0.5f, -0.5f, 0.0f, // right
  //    0.0f,  0.5f, 0.0f  // top
  // };
 
  float vertices[] = {
     // position        // color
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // top
  };

  uint32_t VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // only position attributes from `layout (location = 0) in vec3 aPos;`
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  // glEnableVertexAttribArray(0);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // just demostrate, VAO is already bound.
  glBindVertexArray(VAO);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // render background, rgbo (opaque)
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    // change the color of the triangle
    float timeValue = (float)glfwGetTime();
    float greenValue = std::sin(timeValue) / 2.0f + 0.5f;
    int vertexColorLoation = glGetUniformLocation(shaderProgram, "ourColor");
    glUniform4f(vertexColorLoation, 0.0f, greenValue, 0.0f, 1.0f);

    // render the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
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