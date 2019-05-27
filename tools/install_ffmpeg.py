from zipfile import ZipFile
from pathlib import Path
from utils import download, remove_unnecessary_files_or_dirs
import os
import shutil

def get_dirname_from_link(link):
    return link[link.rfind('/')+1:link.rfind('.')]

def prepare_ffmpeg(out_dir=None, version=None):
    ffmpeg_version = "4.1.3"
    output_dir = os.getcwd() + "/test/"
    
    ffmpeg_win32_dev_url = "https://ffmpeg.zeranoe.com/builds/win32/dev/ffmpeg-" + ffmpeg_version + "-win32-dev.zip"
    ffmpeg_win64_dev_url = "https://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-" + ffmpeg_version + "-win64-dev.zip"
    ffmpeg_win32_shared_url = "https://ffmpeg.zeranoe.com/builds/win32/shared/ffmpeg-" + ffmpeg_version + "-win32-shared.zip"
    ffmpeg_win64_shared_url = "https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-" + ffmpeg_version + "-win64-shared.zip"

    pre_downloads = [
        ffmpeg_win32_dev_url,
        ffmpeg_win64_dev_url,
        ffmpeg_win32_shared_url,
        ffmpeg_win64_shared_url
    ]

    wanted_downloads = {}
    for d in pre_downloads:
        wanted_downloads[d] = d[d.rfind('/')+1:]

    for url, filename in wanted_downloads.items():
        if not Path(output_dir + filename).is_file() or os.stat(output_dir + filename).st_size == 0:
            print("Downloading " + url + " ...")
            download(url, output_dir + filename)
            print("Extract " + filename + " ...")
            ZipFile(output_dir + filename, 'r').extractall(output_dir)

    # Copy include headers
    shutil.copytree(output_dir + get_dirname_from_link(ffmpeg_win32_dev_url) + "/include", output_dir + "/include/ffmpeg")
    # Copy libraries
    shutil.copytree(output_dir + get_dirname_from_link(ffmpeg_win32_dev_url) + "/lib", output_dir + "/lib/ffmpeg/win32")
    shutil.copytree(output_dir + get_dirname_from_link(ffmpeg_win64_dev_url) + "/lib", output_dir + "/lib/ffmpeg/win64")
    # Copy dlls
    shutil.copytree(output_dir + get_dirname_from_link(ffmpeg_win32_shared_url) + "/bin", output_dir + "/bin/win32")
    shutil.copytree(output_dir + get_dirname_from_link(ffmpeg_win64_shared_url) + "/bin", output_dir + "/bin/win64")

    unnecessary_zip_files = []
    unnecessary_dirs = []
    for url, zip_file in wanted_downloads.items():
        unnecessary_zip_files.append(output_dir + zip_file)
        unnecessary_dirs.append(
            output_dir + zip_file[:zip_file.rfind('.')])

    remove_unnecessary_files_or_dirs(
            unnecessary_zip_files, unnecessary_dirs)
