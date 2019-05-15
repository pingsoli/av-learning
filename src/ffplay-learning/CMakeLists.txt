cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} \
        /SAFESEH:NO \
        /SUBSYSTEM:CONSOLE \
        /NODEFAULTLIB:library")
    add_compile_options(/wd4819 /wd4244 /wd4018 /wd4090 /wd4305 /wd4996) # disable warning C4819 under MSVC
endif()

set(MY_ROOT_PATH "${CMAKE_SOURCE_DIR}/../..")
set(MY_INCLUDE_PATH "${MY_ROOT_PATH}/include")
set(MY_LIBRARY_PATH "${MY_ROOT_PATH}/lib")
set(MY_OUTPUT_DIRECTORY "${MY_ROOT_PATH}/bin")

set(FFMPEG_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/ffmpeg")
set(FFMPEG_LIBRARY_PATH "${MY_LIBRARY_PATH}/ffmpeg")

set(SDL_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/sdl2" )
set(SDL_LIBRARY_PATH "${MY_LIBRARY_PATH}/sdl2")

set(LIBYUV_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/libyuv")
set(LIBYUV_LIBRARY_PATH "${MY_LIBRARY_PATH}/libyuv")

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    # 64 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win64")
    set(FFMPEG_LIBRARY_DIRECTORY "${FFMPEG_LIBRARY_PATH}/win64")
    set(SDL_LIBRARY_DIRECTORY "${SDL_LIBRARY_PATH}/win64")
else()
    # 32 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win32")
    set(FFMPEG_LIBRARY_DIRECTORY "${FFMPEG_LIBRARY_PATH}/win32")
    set(SDL_LIBRARY_DIRECTORY "${SDL_LIBRARY_PATH}/win32")
endif()

# Remove 'Debug' and 'Release' directory when building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(
    ${FFMPEG_INCLUDE_DIRECTORY}
    ${SDL_INCLUDE_DIRECTORY}
)

link_directories(
    ${FFMPEG_LIBRARY_DIRECTORY}
    ${SDL_LIBRARY_DIRECTORY}
)

add_executable(ffplay ffplay.c cmdutils.c)

target_link_libraries(
    ffplay
    avformat avcodec avfilter avutil swresample swscale
    SDL2
)