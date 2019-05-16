### Project Introduction

```
the project directory hierarchy
|-- bin            output executable files and dll files(dynamic libraries) location.
|   |-- win32
|   |-- win64
|-- include
|   |-- ffmpeg
|   |-- sdl2
|   |-- ...
|-- lib
|   |-- ffmpeg
|   |   |-- win32
|   |   |-- win64
|   |-- sdl2
|   |   |-- ...
|   |-- ...
|-- src            all codes(projects) are here.
```

NOTE: Only commit the `src` directory, others are ignored.  
On linux platform, don't have to move all dynamic libraries to current project directory,
just specify the path on configuration file(such as *.pro file).  
On windows platform, for convenience, copy all .dlls to `bin/win32` or `bin/win64` directory, and you can
test and debug the program directly.