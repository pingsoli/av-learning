### Project Introduction

```
the project directory
|- bin            output executable files and dll files. (dynamic library)
|   |- win32
|   |- win64
|- include        the directory contains ffmpeg header files.
|   |- ffmpeg
|- lib            the ffmpeg library files.
|   |- win32
|   |- win64
|-src             all codes are here.
```

NOTE: Only commiting the `src` directory, others are ignored.  
On linux platform, you don't have to move all dynamic libraries to current project,
just specify the path on configuration file(such as *.pro file).  
On windows platform, for convenience, we copy all dlls to `bin/win32` or `bin/win64` directory.
we can test and debug the program directly, however, you also can specify the system environment
path according to fit your requirement.