### Project Introduction

```
the project directory
|-- bin            output executable files and dll files. (dynamic library)
|   |-- win32
|   |-- win64
|-- include        the directory contains ffmpeg header files.
|   |-- ffmpeg
|   |-- sdl2
|-- lib
|   |-- ffmpeg
|   |   |-- win32
|   |   |-- win64
|   |-- sdl2
|   |   |-- win32
|   |   |-- win64
|-- src            all codes(projects) are here.
```

NOTE: Only commit the `src` directory, others are ignored.  
On linux platform, don't have to move all dynamic libraries to current project directory,
just specify the path on configuration file(such as *.pro file).  
On windows platform, for convenience, copy all .dlls to `bin/win32` or `bin/win64` directory, and you can
test and debug the program directly.