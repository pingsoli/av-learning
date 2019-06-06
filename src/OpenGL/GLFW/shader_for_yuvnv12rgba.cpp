// Shader convert yuv, nv12 to rgb to render
// NOTE: Converting yuv to rgb may cause pictures lose quality,
// the picture is washed out. the reason is yuv(16-235) value to rgb(0-255)
// value losing precision, it's inevitable.
//
// references
// 1. https://hg.libsdl.org/SDL/file/1f2cb42aa5d3/src/render/opengl/SDL_shaders_gl.c#l128

#include "utils.h"

#include <fstream>
#include <sstream>
#include <string>

const char *vertexShaderSource =
  "in vec3 position;\n"
  "in vec2 texCoord;\n"
  "out vec2 TexCoord;\n"
  "void main(void)\n"
  "{\n"
  "  gl_Position = vec4(position.x, -position.y, position.z, 1.0);\n"
  "  TexCoord = texCoord;\n"
  "}\n";

enum class SamplerType {
  YUV = 0,
  NV12,
  RGBA
};

const char *fragmentShaderSource =
  "in vec2 TexCoord;\n"
  "uniform sampler2D samplerY;\n"
  "uniform sampler2D samplerU;\n"
  "uniform sampler2D samplerV;\n"
  "uniform sampler2D samplerUV;\n"
  "uniform sampler2D samplerRGBA;\n"
  "uniform int samplerType;\n"
  "out vec4 FragColor;\n"
  "void main(void)\n"
  "{\n"
  //                        first column   second column           third column
  // "  mat3 yuv2rgb = mat3(vec3(1, 1, 1), vec3(0, -0.344, 1.772), vec3(1.402, -0.714, 0));\n"
  "  mat3 yuv2rgb = mat3(vec3(1.164, 1.164, 1.164), vec3(0.0, -0.391, 2.018), vec3(1.596, -0.813, 0.0));\n"
  "  vec3 yuv, rgb;\n"
  "  if (samplerType == 0) {\n"
  "    yuv.x = texture(samplerY, TexCoord).r;\n"
  "    yuv.y = texture(samplerU, TexCoord).r - 0.5;\n"
  "    yuv.z = texture(samplerV, TexCoord).r - 0.5;\n"
  "    rgb = yuv2rgb * yuv;\n"
  "    FragColor = vec4(rgb, 1.0);\n"
  "  } else if (samplerType == 1) {\n"
  "    yuv.x = texture(samplerY, TexCoord).r;\n"
  "    yuv.yz = texture(samplerUV, TexCoord).rg - 0.5;\n"
  "    rgb = yuv2rgb * yuv;\n"
  "    FragColor = vec4(rgb, 1.0);\n"
  "  } else if (samplerType == 2) {\n"
  "    FragColor = texture(samplerRGBA, TexCoord).rgba;\n"
  "  }\n"
  "}\n";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main(int argc, char *argv[])
{
  // const int width = 640;
  // const int height = 360;

  const int width = 1280;
  const int height = 720;
  const char filename[] =
      // "test-output-640x360.yuv";
      "test-output-1280x720.yuv";
      // "test-output-640x360.nv12";
      // "test-output-640x360.rgba";
  SamplerType samplerType =
      SamplerType::YUV;
      // SamplerType::NV12;
      // SamplerType::RGBA;
  GLFWwindow* window = createWindow(width, height, "OpenGL Learning", framebuffer_size_callback);

  uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  uint32_t shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glBindAttribLocation(shaderProgram, 0, "position");
  glBindAttribLocation(shaderProgram, 1, "texCoord");

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

  // preload all sampler type(yuv, nv12, rgb)
  uint32_t uniform1i_yuv[3] = { 0 };
  const char *uniform1i_yuv_name[3] = {
    "samplerY", "samplerU", "samplerV"
  };
  for (int i = 0; i < 3; ++i) {
    uniform1i_yuv[i] = glGetUniformLocation(shaderProgram, uniform1i_yuv_name[i]);
  }

  uint32_t uniform1i_nv12[2] = { 0 };
  const char *uniform1i_nv12_name[2] = {
    "samplerY", "samplerUV"
  };
  for (int i = 0; i < 2; ++i) {
    uniform1i_nv12[i] = glGetUniformLocation(shaderProgram, uniform1i_nv12_name[i]);
  }

  const char *uniform1i_rgba_name = "samplerRGBA";
  uint32_t uniform1i_rgba = glGetUniformLocation(shaderProgram, uniform1i_rgba_name);
  glUseProgram(shaderProgram);

  std::ifstream infile(filename, std::ios::binary);
  std::stringstream ss;
  ss << infile.rdbuf();
  std::string data = ss.str();

  const char* yuv[3] = { 0 };
  const char* nv12[2] = { 0 };
  const char* rgba;

  uint32_t sampleTypeLocation = glGetUniformLocation(shaderProgram, "samplerType");
  glUniform1i(sampleTypeLocation, (int)samplerType);

  // YUV texture
  uint32_t yuv_texture_ids[3] = { 0 };
  for (int i = 0; i < 3; ++i) {
    glGenTextures(1, &yuv_texture_ids[i]);
    glBindTexture(GL_TEXTURE_2D, yuv_texture_ids[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  // NV12 texture
  uint32_t nv12_texture_ids[2] = { 0 };
  for (int i = 0; i < 2; ++i) {
    glGenTextures(1, &nv12_texture_ids[i]);
    glBindTexture(GL_TEXTURE_2D, nv12_texture_ids[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  
  // RGBA texture
  uint32_t rgbaTextureId;
  glGenTextures(1, &rgbaTextureId);
  glBindTexture(GL_TEXTURE_2D, rgbaTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    processInput(window);

    switch (samplerType)
    {
      case SamplerType::YUV:
        // YUV420p data
        yuv[0] = &data[0];
        yuv[1] = yuv[0] + width * height;
        yuv[2] = yuv[1] + width * height / 4;

        for (int i = 0; i < 3; ++i)
        {
          glActiveTexture(GL_TEXTURE0 + i);
          glBindTexture(GL_TEXTURE_2D, yuv_texture_ids[i]);
          // @question: why GL_RED could not be replaced by GL_LUMINANCE ?
          // GL_LUMILANCE is deprecated in OpenGL 3.1, use GL_RED for single value,
          // GL_RGBA for a (r, g, b, a) group.
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                       (i > 0 ? width / 2 : width), (i > 0 ? height / 2 : height),
                       0, GL_RED, GL_UNSIGNED_BYTE, yuv[i]);
          glUniform1i(uniform1i_yuv[i], i);
        }
        break;

      case SamplerType::NV12:
        nv12[0] = &data[0];
        nv12[1] = nv12[0] + width * height;

        for (int i = 0; i < 2; ++i) {
          glActiveTexture(GL_TEXTURE0 + i);
          glBindTexture(GL_TEXTURE_2D, nv12_texture_ids[i]);
          glTexImage2D(GL_TEXTURE_2D, 0,
                       (i > 0 ? GL_RG : GL_RED),
                       (i > 0 ? width / 2 : width), (i > 0 ? height / 2 : height),
                       0,
                       (i > 0 ? GL_RG : GL_RED),
                       GL_UNSIGNED_BYTE,
                       nv12[i]);
          glUniform1i(uniform1i_nv12[i], i);
        }
        break;

      case SamplerType::RGBA:
        rgba = &data[0];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rgbaTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        glUniform1i(uniform1i_rgba, 0);
        break;
    }

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

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}