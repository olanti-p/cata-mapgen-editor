#!/usr/bin/env python3

import zipfile
import os

def zip_file( zipf, src, dst = None ):
    if dst is None:
        dst = src
    zipf.write(src, dst)


def zip_dir( zipf, src, dst = None ):
    if dst is None:
        dst = src
    for root, dirs, files in os.walk(src):
        for file in files:
            zip_file(zipf, os.path.join(root, file), os.path.join(dst, file))
            #ziph.write(os.path.join(root, file), 
            #           os.path.relpath(os.path.join(root, file), 
            #                           os.path.join(src, '..')))


with zipfile.ZipFile('bnmt-bindist.zip', 'w', zipfile.ZIP_DEFLATED) as zipf:
    if os.path.isfile('build/src/cataclysm-tiles'):
        zip_file(zipf, 'build/src/cataclysm-tiles', './cataclysm-bnmt')
    else:
        zip_file(zipf, 'build/src/cataclysm-tiles.exe', './cataclysm-bnmt.exe')
    zip_file(zipf, 'README.md')
    zip_file(zipf, 'data/raw/keybindings/bnmt.json')
    zip_dir(zipf, 'data/mods/_me_interface/')
    zip_dir(zipf, 'doc/BNMT')
