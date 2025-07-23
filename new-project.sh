#!/bin/bash

path=$(pwd)
project=${path##*/}
commandCount=0
optionCount=0
override=0

glslangName=glslang-main-linux-Release
if [[ $OSTYPE == "mysys" ]]; then
	glslangName=glslang-master-windows-Release
fi

CreateDirectories ()
{
	echo "CREATING DIRECTORIES"

	if ! test -d $path/includes; then
		mkdir includes
	fi

	if ! test -d $path/sources; then
		mkdir sources
	fi

	if ! test -d $path/shaders; then
		mkdir shaders
	fi

	echo "DIRECTORIES CREATED"
}

FetchSetup ()
{
	if [[ $override == 1 ]]; then
		rm -f $path/setup.sh
	fi

	if ! test -f $path/setup.sh; then
		echo "FETCHING SETUP SCRIPT"
		wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/setup.sh
		sed -i "s/^project=[^ ]*/project="$project"/" $path/setup.sh
		echo "SETUP SCRIPT FETCHED"
	fi
}

FetchCMakeFile ()
{
	if [[ $override == 1 ]]; then
		rm -f $path/CMakeLists.txt
	fi

	if ! test -f $path/CMakeLists.txt; then
		echo "FETCHING CMAKE FILE"
		wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/CMakeListsNewProject.txt
		mv CMakeListsNewProject.txt CMakeLists.txt
		sed -i "s/^project([^ ]*/project("$project" VERSION 1.0)/" $path/CMakeLists.txt
		echo "CMAKE FILE FETCHED"
	fi
}

FetchShaderCompilers ()
{
	if [[ $override == 1 ]]; then
		rm -f $path/shader-compiler.sh
		rm -f $path/glslang
	fi

	if ! test -f $path/shader-compiler.sh; then
		echo "FETCHING SHADER COMPILER SCRIPT"
		wget https://raw.githubusercontent.com/ferrefire/Limcore/refs/heads/main/shader-compiler.sh
		echo "SHADER COMPILER SCRIPT FETCHED"
	fi

	if ! test -f $path/glslang; then
		echo "FETCHING GLSLANG EXECUTABLE"
		wget https://github.com/KhronosGroup/glslang/releases/download/main-tot/$glslangName.zip
		unzip -d $path/$glslangName $path/$glslangName.zip
		mv $path/$glslangName/bin/glslang $path/glslang
		rm -rf $path/$glslangName.zip
		rm -rf $path/$glslangName
		echo "GLSLANG EXECUTABLE FETCHED"
	fi
}

CleanDirectories ()
{
	echo "DELETING SOURCE DIRECTORIES"
	rm -rf $path/includes
	rm -rf $path/sources
	rm -rf $path/shaders
	echo "SOURCE DIRECTORIES DELETED"
}

CleanFetches ()
{
	echo "DELETING FETCHED FILES"
	rm -f $path/setup.sh
	rm -f $path/CMakeLists.txt
	rm -f $path/shader-compiler.sh
	rm -f $path/glslang
	echo "FETCHED FILES DELETED"
}

ExecuteCommand ()
{
	if [[ $1 == "dirs" ]] || [[ $1 == "d" ]]; then
		CreateDirectories
	elif [[ $1 == "setup" ]] || [[ $1 == "s" ]]; then
		FetchSetup
	elif [[ $1 == "cmk" ]] || [[ $1 == "c" ]]; then
		FetchCMakeFile
	elif [[ $1 == "shdrcmp" ]] || [[ $1 == "g" ]]; then
		FetchShaderCompilers
	fi
}

Clean ()
{
	if read -p "Delete all source directories? (y/n): " confirmDir && [[ $confirmDir == [yY] ]]; then
		CleanDirectories
	fi
	echo ""

	if read -p "Delete all fetched files? (y/n): " confirmFetched && [[ $confirmFetched == [yY] ]]; then
		CleanFetches
	fi
	echo ""

	if read -p "Exit? (y/n): " confirmExit && [[ $confirmExit == [yY] ]]; then
		exit 1
	fi
	echo ""
}

ParseOption ()
{
	if [[ $1 == "-override" ]] || [[ $1 == "-o" ]]; then
		override=1
	elif [[ $1 == "-clean" ]] || [[ $1 == "-c" ]]; then
		Clean
	fi
}

for x in $@; do
	if [[ $x == "-"* ]]; then
		ParseOption $x
		((optionCount+=1))
	fi
done

for x in $@; do
	if [[ $x != "-"* ]]; then
		ExecuteCommand $x
		((commandCount+=1))
	fi
done

if test $commandCount == 0; then
	ExecuteCommand d
	ExecuteCommand s
	ExecuteCommand c
	ExecuteCommand g
fi