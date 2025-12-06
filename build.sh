#!/bin/bash
rm -rf build
rm -r ./compile_commands.json

mkdir build && cd build
cmake ..
ln -sf ./build/compile_commands.json ..