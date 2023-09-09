#!/bin/bash
direnv allow .
source .direnv/**/bin/activate
pip install -U -r requirements-dev.txt

wget https://github.com/OpenCyphal/public_regulated_data_types/archive/refs/heads/master.zip -O dsdl.zip -q --show-progress
mkdir -p ~/.cyphal
unzip -uq dsdl.zip -d ~/.cyphal
mv -f ~/.cyphal/public_regulated_data_types-*/* --target-directory=${HOME}/.cyphal/
rm -rf ~/.cyphal/public_regulated_data_types-*
rm dsdl.zip 