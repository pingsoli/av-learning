cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(MY_ROOT_PATH "${CMAKE_SOURCE_DIR}/../..")
set(MY_INCLUDE_PATH "${MY_ROOT_PATH}/include")
set(MY_LIBRARY_PATH "${MY_ROOT_PATH}/lib")
set(MY_OUTPUT_DIRECTORY "${MY_ROOT_PATH}/bin")

set(FFMPEG_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/ffmpeg")
set(FFMPEG_LIBRARY_PATH "${MY_LIBRARY_PATH}/ffmpeg")

set(SDL_INCLUDE_DIRECTORY "${MY_INCLUDE_PATH}/sdl2" )
set(SDL_LIBRARY_PATH "${MY_LIBRARY_PATH}/sdl2")

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    # 64 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win64")
    set(FFMPEG_LIBRARY_DIRECTORY "${FFMPEG_LIBRARY_PATH}/win64")
    set(SDL_LIBRARY_DIRECTORY "${SDL_LIBRARY_PATH}/win64")
    set(QT_ROOT_PATH "D:/Qt/5.12.3/msvc2017_64")
else()
    # 32 bit environment
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MY_OUTPUT_DIRECTORY}/win32")
    set(FFMPEG_LIBRARY_DIRECTORY "${FFMPEG_LIBRARY_PATH}/win32")
    set(SDL_LIBRARY_DIRECTORY "${SDL_LIBRARY_PATH}/win32")
    set(QT_ROOT_PATH "D:/Qt/5.12.3/msvc2017")
endif()

# Remove 'Debug' and 'Release' directory when building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(simplest_yuv420p_split simplest_yuv420p_split.c)
add_executable(simplest_yuv444p_split simplest_yuv444p_split.c)
add_executable(simplest_yuv420p_gray simplest_yuv420p_gray.c)
add_executable(simplest_yuv420p_y_half simplest_yuv420p_y_half.c)
add_executable(simplest_yuv420p_border simplest_yuv420p_border.c)

add_executable(h264_parser h264_parser.cpp)
add_executable(rtsp_over_tcp_h264 rtsp_over_tcp_h264.cpp)
if (WIN32)
    target_link_libraries(rtsp_over_tcp_h264 ws2_32)
endif (WIN32)

add_executable(aac_parser aac_parser.cpp)

add_executable(flv_parser flv_parser.cpp)
if (WIN32)
    target_link_libraries(flv_parser ws2_32)
endif (WIN32)

add_executable(rtp_ts_parser rtp_ts_parser.cpp)
add_executable(mp3_parser mp3_parser.cpp)

include_directories(
    ${SDL_INCLUDE_DIRECTORY}
)

link_directories(
    ${SDL_LIBRARY_DIRECTORY}
)

add_executable(wav_player wav_player.cpp)
# target_link_directories(wav_player
#     PRIVATE
#     ${SDL_LIBRARY_DIRECTORY}
# )
# target_include_directories(wav_player
#     PRIVATE
#     ${SDL_INCLUDE_DIRECTORY}
# )
set_property(
    TARGET wav_player
    PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
)

add_executable(pcm_player_with_sdl pcm_player_with_sdl.cpp)

set(CMAKE_PREFIX_PATH ${QT_ROOT_PATH})
find_package(Qt5Core)
add_executable(pcm_player_with_qt pcm_player_with_qt.cpp)
qt5_use_modules(pcm_player_with_qt Core Widgets Multimedia)