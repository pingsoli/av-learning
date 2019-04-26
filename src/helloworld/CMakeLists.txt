cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Debug)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

set(MY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../../bin")

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    # 64 bit
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win64")
    set(QT_ROOT_PATH "D:\\Qt\\5.12.3\\msvc2017_64\\")
else()
    # 32 bit
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win32")
    set(QT_ROOT_PATH "D:\\Qt\\5.12.3\\msvc2017\\")
endif()

# Remove 'Debug' and 'Release' directory when building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_PREFIX_PATH ${QT_ROOT_PATH})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)

add_executable(main main.cpp widget.cpp)
qt5_use_modules(main Widgets)