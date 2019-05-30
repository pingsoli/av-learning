#include "utils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// const char *vertexShaderSource =
//     "layout (location = 0) in vec3 aPos;"
//     "layout (location = 1) in vec3 aColor;"
//     "out vec3 ourColor;"
//     "uniform float offset;"
//     "void main()"
//     "{"
//     "  gl_Position = vec4(aPos.x + offset, aPos.y, aPos.z, 1.0);"
//     "  ourColor = aColor;"
//     "}";

// const char *fragmentShaderSource =
//     "out vec4 FragColor;"
//     "in vec3 ourColor;"
//     "void main()"
//     "{"
//     "  FragColor = vec4(ourColor, 1.0);"
//     "}";

const char *vertexShaderSource =
    "layout (location = 0) in vec3 aPos;"
    "layout (location = 1) in vec3 aColor;"
    "out vec3 ourPosition;"
    "void main()"
    "{"
    "  gl_Position = vec4(aPos, 1.0);"
    "  ourPosition = aPos;"
    "}";

const char *fragmentShaderSource =
    "out vec4 FragColor;"
    "in vec3 ourPosition;"
    "void main()"
    "{"
    "  FragColor = vec4(ourPosition, 1.0);"
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

  float x_offset = 0.0f;
  float x_stride = 0.01f;
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

    // int offsetLocation = glGetUniformLocation(shaderProgram, "offset");
    // // if (x_offset >= 1.5) x_offset = -x_offset;
    // // x_offset += 0.01f;

    // // collision
    // if (x_offset >= 0.5 || x_offset <= -0.5) x_stride = -x_stride;
    // x_offset += x_stride;

    // glUniform1f(offsetLocation, x_offset);

    // render the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();

#if (_WIN32 || _WIN64)
    Sleep(30); // 1/30 = 33.333.. fps
#endif
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