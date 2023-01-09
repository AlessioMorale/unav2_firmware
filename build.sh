#!/bin/bash
mkdir -p build && cd build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=true -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make all -j $@
