#!/bin/sh
# Build the Holy Bible on Linux / macOS.
set -e
cd "$(dirname "$0")"
${CXX:-g++} -std=c++17 -O2 -o holybible src/main.cpp
echo "Built ./holybible  -  run it with:  ./holybible"
