cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:library")
endif()

set(MY_ROOT_PATH "${CMAKE_SOURCE_DIR}/../../..")
set(MY_INCLUDE_PATH "${MY_ROOT_PATH}/include")
set(MY_LIBRARY_PATH "${MY_ROOT_PATH}/lib")
set(MY_OUTPUT_DIRECTORY "${MY_ROOT_PATH}/bin")

set(GLFW_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/glfw")
set(GLFW_LIBRARY_PATH "${MY_LIBRARY_PATH}/glfw")

# stab is header-only library
set(STAB_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/stab")

# glm is header-only library
set(GLM_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/glm")

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    # 64 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win64")
    set(GLFW_LIBRARY_DIRECTORY "${GLFW_LIBRARY_PATH}/win64")
else()
    # 32 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win32")
    set(GLFW_LIBRARY_DIRECTORY "${GLFW_LIBRARY_PATH}/win32")
endif()

# Remove 'Debug' and 'Release' directory when building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(
  ${GLFW_INCLUDE_DIRECTORY}
  ${STAB_INCLUDE_DIRECTORY}
  ${GLM_INCLUDE_DIRECTORY}
)

link_directories(
  ${GLFW_LIBRARY_DIRECTORY}
)

file(GLOB src_files
    "*.cpp")

foreach(file ${src_files})
    get_filename_component(filename ${file} NAME)
    get_filename_component(target_name ${file} NAME_WE)
    add_executable(${target_name} ${filename} glad.c)
    set_property(
        TARGET ${target_name}
        PROPERTY
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
    )
    target_link_libraries(${target_name} glfw3)
endforeach()