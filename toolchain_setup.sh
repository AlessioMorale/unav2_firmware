#!/bin/bash
set -ex

direnv allow .

./tools/local_toolchain_setup.sh
./tools/local_cmake_setup.sh "3.22.2"
