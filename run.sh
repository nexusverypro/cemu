#!/usr/bin/sh
cd ./src
g++ -std=c++23 -g ./main.cpp ./ilbuilder.cpp ./binary.cpp -o ../out/cemu
cd ../
./out/cemu