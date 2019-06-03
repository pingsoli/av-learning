#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void generateTexture(const char* filename);

const char *vertexShaderSource =
  "#version 440 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "layout (location = 1) in vec2 aTexCoord;\n"
  "out vec2 TexCoord;\n"
  "uniform mat4 transform;\n"
  "void main(void)\n"
  "{\n"
  "  gl_Position = transform * vec4(aPos, 1.0);\n"
  // "  gl_Position = vec4(aPos, 1.0);\n"
  "  TexCoord = aTexCoord;\n"
  "}\n";

// linearly interpolate between both textures (80% container, 20% awesomeface)
const char *fragmentShaderSource =
  "#version 440 core\n"
  "out vec4 FragColor;\n"
  "in vec2 TexCoord;\n"
  "uniform sampler2D texture1;\n"
  "uniform sampler2D texture2;\n"
  "void main(void)\n"
  "{\n"
  "  FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
  "}\n";

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
    // position          // texture coords
    0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
    0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
   -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
   -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, // top left
  };

  unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

  uint32_t VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  // texture attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  uint32_t texture1;
  const char bottom_pic_filename[] = "test-output.png";
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  // setting the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // setting the texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  generateTexture(bottom_pic_filename);

  uint32_t texture2;
  const char top_pic_filename[] = "logo.jpg";
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  // setting the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // setting the texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  generateTexture(top_pic_filename);

  glUseProgram(shaderProgram);
  glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
  glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind texture1 and texture2 separately.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // first transformation
    // create transform
    glm::mat4 transform = glm::mat4(1.0f);
    // second parameter is the offset 
    transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
    // third parameter is ratate around x, y or z axis.
    transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

    glUseProgram(shaderProgram);
    uint32_t transformLocation = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));

    // render first transformation
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indices, not 4

    // second transformation
    transform = glm::mat4(1.0f); // reset it to identity matrix
    transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, 0.0f));
    float scaleAmount = sin(glfwGetTime());
    transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, &transform[0][0]);

    // render second transformation
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    Sleep(30);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
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

void generateTexture(const char* filename) {
  int width, height, channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}