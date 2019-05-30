// exercises for drawing two triangles
// exercise 1: draw two triangles using same VAO and VBO
// exercise 2: draw two triangles using different VAOs and VBOs

#include "utils.h"
#include <iostream>

// OpenGL Shading Language
const char *vertexShaderSource =
    "layout (location = 0) in vec3 aPos;"
    "void main()"
    "{"
    "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
    "}";

const char *fragmentShaderSource =
    "out vec4 FragColor;"
    "void main()"
    "{"
    "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
    "}";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main(int argc, char *argv[])
{
  const unsigned int width = 800;
  const unsigned int height = 600;

  GLFWwindow *window = createWindow(width, height, "OpenGL Excercises", framebuffer_size_callback);
  // Build and compile our shader program
  // vertex shader
  unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  // fragment shader
  unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  // link shaders
  int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // checkStatus(shaderProgram, GL_LINK_STATUS);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  ////////////////////////////////////////////////////////////////////////////////////
  // // Exercise 1: draw two triangles next to each other
  // float vertices[] = {
  //   -0.5f, 0.5f, 0.0f,
  //   -0.5f, 0.0f, 0.0f,
  //    0.0f, 0.0f, 0.0f,
  //    0.0f, 0.5f, 0.0f,
  //    0.5f, 0.0f, 0.0f
  // };
  // unsigned int indices[] = {
  //   0, 1, 2, // first triangle
  //   2, 3, 4  // second triangle
  // };

  // unsigned int EBO, VBO, VAO;
  // glGenVertexArrays(1, &VAO);
  // glGenBuffers(1, &EBO);
  // glGenBuffers(1, &VBO);

  // // bind the vertex array object first, then bind and set vertex buffer, and then
  // // configure vertex attributes.
  // glBindVertexArray(VAO);

  // glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  // glEnableVertexAttribArray(0);

  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  // // Do not unbind EBO when VAO is active
  // // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // glBindVertexArray(0);
  ////////////////////////////////////////////////////////////////////////////////////

  // Exercise 2: create 2 triangles using two different VAOs and VBOs
  float vertices1[] = { // first triangle
    -0.5f, 0.5f, 0.0f,
    -0.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f,
  };
  float vertices2[] = { // second triangle
    0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f,
    0.5f, 0.0f, 0.0f
  };

  unsigned int VAO[2], VBO[2];
  glGenVertexArrays(2, VAO);
  glGenBuffers(2, VBO);

  glBindVertexArray(VAO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(VAO[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  ////////////////////////////////////////////////////////////////////////////////////

  // uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // draw first triangle using the data from VAO[0]
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // then we draw the second triangle using the data from VAO[1]
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(2, VAO);
  glDeleteBuffers(2, VBO);

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