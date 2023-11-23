#!/bin/bash
set -ex

pip install -U -r requirements-dev.txt

curl -sSL "https://github.com/OpenCyphal/public_regulated_data_types/archive/refs/heads/master.zip" -o dsdl.zip
mkdir -p ~/.cyphal
unzip -uq dsdl.zip -d ~/.cyphal
mv -f ~/.cyphal/public_regulated_data_types-*/* "${HOME}/.cyphal/"
rm -rf ~/.cyphal/public_regulated_data_types-*
rm dsdl.zip
