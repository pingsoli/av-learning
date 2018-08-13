TEMPLATE = app
CONFIG += console c++11
SOURCES += main.cpp

INCLUDEPATH += $$PWD/../../include/ffmpeg

# check qtcreator inner variable
# message($$QMAKESPEC)


win32 {
  opt = $$find(QMAKESPEC, "msvc2015_64")
  isEmpty(opt) {
    message("using win32 library")
    LIBS += -L$$PWD/../../lib/win32
  }
  
  !isEmpty(opt) {
    message("using win64 library")
    LIBS += -L$$PWD/../../lib/win64
  }
}


linux {
  message("linux platform")
  
  LIBS += -L/usr/local/ffmpeg/lib -lavcodec -lavutil -lswresample
}
