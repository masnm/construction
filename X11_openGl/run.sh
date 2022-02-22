#!/bin/bash

set -xe

if [ "$1" == "" ]; then
	exit 1
fi

g++ $1.cpp -lX11 -lpthread -lGL -lGLU -std=c++17
./a.out
rm a.out
