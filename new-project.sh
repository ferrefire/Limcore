#!/bin/bash

wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/setup.sh
wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/shader-compiler.sh
wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/CMakeListsNewProject.txt

mv CMakeListsNewProject.txt CMakeLists.txt

mkdir includes
mkdir sources