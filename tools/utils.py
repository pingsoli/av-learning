from requests import get
import os
import shutil
from pathlib import Path

def download(url, outfile):
    with open(outfile, 'wb') as out_file:
        out_file.write(get(url).content)

def remove_unnecessary_files_or_dirs(files, dirs):
    for f in files:
        if Path(f).is_file():
            os.remove(f)
    for d in dirs:
        if Path(d).is_dir():
            shutil.rmtree(d)