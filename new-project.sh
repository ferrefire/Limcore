#!/bin/bash

path=$(pwd)

glslangName=glslang-main-linux-Release
if [[ $OSTYPE == "mysys" ]]; then
	glslangName=glslang-master-windows-Release
fi

mkdir includes
mkdir sources
mkdir shaders

wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/setup.sh

wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/CMakeListsNewProject.txt
mv CMakeListsNewProject.txt CMakeLists.txt

wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/shader-compiler.sh

wget https://github.com/KhronosGroup/glslang/releases/download/main-tot/$glslangName.zip
unzip -d $path/$glslangName $path/$glslangName.zip
mv $path/$glslangName/bin/glslang $path/glslang
rm -rf $path/$glslangName.zip
rm -rf $path/$glslangName