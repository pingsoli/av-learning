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
to intall GLFW and GALD on windows, you can refer to this [link](https://learnopengl.com/Getting-started).  
GLFW is a library for OpenGL.  
GLAD is extension for using OpenGL correctly.  
**NOTE: don't delete the `glad.c` file in the project.**

---
#### stab library for graphics and audio
git repo: https://github.com/nothings/stb.git  
this library used in OpenGL texture examples, it helped to load a *.png file to raw char data.

stab is header-only library.

---
#### GLM library for OpenGL Mathematics
git repo: https://github.com/g-truc/glm.git  
It's a header-only library.

---
#### Math basisc of matrix
```
Scaling
  | s1  0  0  0 | | x |     | s1 * x |
  | 0  s2  0  0 | | y | ==> | s2 * y |
  | 0   0 s3  0 | | z | ==> | s3 * z |
  | 0   0  0  1 | | 1 |     |    1   |

Translation
  | 1 0 0 a | | x |     | x + a |
  | 0 1 0 b | | y | ==> | y + b |
  | 0 0 1 c | | z | ==> | z + c |
  | 0 0 0 1 | | 1 |     |   1   |

Rotation
  around X-axis
    | 1  0     0   0 | | x |     |        x        |
    | 0 cosθ -sinθ 0 | | y | ==> | cosθ*y - sinθ*z |
    | 0 sinθ cosθ  0 | | z | ==> | sinθ*y + cosθ*z |
    | 0  0     0   1 | | 1 |     |        1        |

  around Y-axis
    |  cosθ 0 sinθ 0 | | x |     | cosθ*x + sinθ*z |
    |   0   1   0  0 | | y | ==> |        y        |
    | -sinθ 0 cosθ 0 | | z | ==> |-sinθ*x + cosθ*z |
    |   0   0   0  1 | | 1 |     |        1        |

  around Z-axis
    | cosθ -sinθ 0 0 | | x |     | cosθ*x - sinθ*y |
    | sinθ  cosθ 0 0 | | y | ==> | sinθ*x + cosθ*y |
    |  0     0   1 0 | | z | ==> |        z        |
    |  0     0   0 1 | | 1 |     |        1        |
```