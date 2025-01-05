#!/bin/bash

cd build
cmake -G Ninja ..
ninja
chmod +x ACMResearch
cd ..

clear
echo "Running ./build/ACMResearch $@"
echo ''

./build/ACMResearch $@