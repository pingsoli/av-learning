#ifndef _UTILS_H_
#define _UTILS_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

GLFWwindow *createWindow(int width, int height, const char* name,
  void (*callback)(GLFWwindow*, int, int))
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, name, NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
  }

  return window;
}

// return shader id
GLuint compileShader(GLenum type, const char* shaderStr)
{
  unsigned int shaderId = glCreateShader(type);
  glShaderSource(shaderId, 1, &shaderStr, nullptr);
  glCompileShader(shaderId);
  int success = 0;
  static char infoLog[512] = { 0 };
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shaderId, sizeof(infoLog), nullptr, infoLog);
    std::cout << "Error:" << infoLog << std::endl;
  }

  return shaderId;
}

#endif