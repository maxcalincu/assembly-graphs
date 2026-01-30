#!/bin/bash
rm -rf build
rm -r ./compile_commands.json

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
ln -sf ./build/compile_commands.json ..