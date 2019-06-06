// Shader don't support YUV directly, we need to transform YUV data to RGB
// we will do the process inner shader, not outside world.

#include "utils.h"

#include <fstream>
#include <string>
#include <sstream>

// NOTE: v-flip the picture
const char *vertexShaderSource =
  "layout (location = 0) in vec3 aPos;\n"
  "layout (location = 1) in vec2 aTexCoord;\n"
  "out vec2 TexCoord;\n"
  "void main(void)\n"
  "{\n"
  "  gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1);\n"
  "  TexCoord = aTexCoord;\n"
  "}\n";

const char *fragmentShaderSource =
  "in vec2 TexCoord;\n"
  "out vec4 FragColor;\n"
  "uniform sampler2D samplerY;\n"
  "uniform sampler2D samplerU;\n"
  "uniform sampler2D samplerV;\n"
  "void main(void)\n"
  "{\n"
  "  vec3 yuv;\n"
  "  vec3 rgb;\n"
  "  yuv.x = texture(samplerY, TexCoord).r;\n"
  "  yuv.y = texture(samplerU, TexCoord).r - 0.5;\n"
  "  yuv.z = texture(samplerV, TexCoord).r - 0.5;\n"
  "  rgb = mat3(1,        1,       1,\n"
  "             0,       -0.39465, 2.032211,\n"
  "             1.13983, -0.58060, 0) * yuv;"
  "  FragColor = vec4(rgb, 1);\n"
  "}\n";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char *argv[])
{
  const int width = 640;
  const int height = 360;
  const char filename[] = "test-output-640x360.yuv";

  GLFWwindow* window = createWindow(width, height, "OpenGL Learning", framebuffer_size_callback);

  uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  uint32_t shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  float vertices[] = {
    // position         // texture coords
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
   -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
   -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top left
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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  uint32_t samplerY = glGetUniformLocation(shaderProgram, "samplerY");
  uint32_t samplerU = glGetUniformLocation(shaderProgram, "samplerU");
  uint32_t samplerV = glGetUniformLocation(shaderProgram, "samplerV");

  uint32_t id_y, id_u, id_v;
  glGenTextures(1, &id_y);
  glBindTexture(GL_TEXTURE_2D, id_y);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &id_u);
  glBindTexture(GL_TEXTURE_2D, id_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &id_v);
  glBindTexture(GL_TEXTURE_2D, id_v);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // read all data from yuv file
  std::ifstream infile(filename, std::ios::binary);
  std::stringstream ss;
  ss << infile.rdbuf();
  std::string data = ss.str();

  const char* yuv[3];
  yuv[0] = &data[0];
  yuv[1] = yuv[0] + width * height;
  yuv[2] = yuv[1] + width * height / 4;

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    // Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, yuv[0]);
    glUniform1i(samplerY, 0);

    // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, yuv[1]);
    glUniform1i(samplerU, 1);

    // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, yuv[2]);
    glUniform1i(samplerV, 2);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indices, not 4

    // render
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}