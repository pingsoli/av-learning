from utils import download
import os

def get_filename_from_link(url):
    return url[url.rfind('/')+1:]

def prepare_sdl(out_dir=None, version=None):
    sdl_version = "2.0.9"
    output_dir = os.getcwd() + "/test/"
    sdl_win32_url = "https://www.libsdl.org/release/SDL2-" + sdl_version + "-win32-x86.zip"
    sdl_win64_url = "https://www.libsdl.org/release/SDL2-" + sdl_version + "-win32-x64.zip"
    sdl_src_url = "https://www.libsdl.org/release/SDL2-" + sdl_version + ".zip"

    downloads = [
        sdl_src_url,
        sdl_win32_url,
        sdl_win64_url
    ]

    for url in downloads: