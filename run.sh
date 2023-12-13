#!/bin/sh
set -e
cmake -B build -DCMAKE_BUILD_TYPE=False
cmake --build build
build/main
