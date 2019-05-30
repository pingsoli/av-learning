### OpenGL

**the outline of pipeline processing**
```
Vertex Data[] -> Vertex Shader -> Shape Assembly -> Geometry Shader ---+
                                                                       |
             Tests and Blending <- Fragment Shader <- Rasterization <--+

Basically, we can only control Vertex Data, Vertex Shader, Geometry Shader and Fragment Shader.
```

**What is shader ?**  
Shader is little program rests on CPU can be executed parallelly.
Shaders are written in C-like language GLSL(OpenGL Shading Language).
there are various types of shader, such as vertex shader and fragment shader.

---
#### GLFW and GLAD
intall GLFW and GALD on windows, you can refer to this [link](https://learnopengl.com/Getting-started).

