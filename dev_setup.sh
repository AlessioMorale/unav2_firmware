#!/bin/bash
python3 -m venv .venv
source .venv/bin/activate
pip install -U -r requirements-dev.txt

mkdir -p ~/.cyphal
wget https://github.com/OpenCyphal/public_regulated_data_types/archive/refs/heads/master.zip -O dsdl.zip
unzip dsdl.zip -d ~/.cyphal
mv -f ~/.cyphal/public_regulated_data_types*/* ~/.cyphal
rm dsdl.zip