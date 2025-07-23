#!/bin/bash

glslangName=glslang-main-linux-Release
if [[ $OSTYPE == "mysys" ]]; then
	glslangName=glslang-master-windows-Release
fi

wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/setup.sh
wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/CMakeListsNewProject.txt
wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/shader-compiler.sh
wget https://github.com/KhronosGroup/glslang/releases/download/main-tot/$glslangName.zip

mv CMakeListsNewProject.txt CMakeLists.txt

unzip $glslangName.zip
mv $glslangName/bin/glslang ./glslang
rm -rf $glslangName.zip
rm -rf $glslangName

mkdir includes
mkdir sources
mkdir shaders