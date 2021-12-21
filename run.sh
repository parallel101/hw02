#!/bin/sh
set -e
cmake -B build
cmake --build build
build/bin/main
read -p "Press any key to resume ..."