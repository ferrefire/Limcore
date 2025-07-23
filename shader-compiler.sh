#!/bin/bash

path=$(pwd)
glslang_path=$path/build/_deps/glslang-build/StandAlone/glslang
if [[ $OSTYPE == "msys" ]]; then
	glslang_path=$path/build/_deps/glslang-build/StandAlone/Release/glslang
fi

while ! test -d $path/shaders; do
	cd ..
	path=$(pwd)
done

if ! test -f $glslang_path; then
	echo "ERROR: glslang not found."
	exit 1
fi

compile_shader () 
{
	"$glslang_path" -V $1 -o $1.spv
}

if [[ $@ == "ALL" ]] || [[ $@ == "all" ]]; then
	files="$path/shaders/*"
	for f in $files; do
		if [[ $f != *".spv" ]] && [[ $f != *".glsl" ]]; then
			compile_shader $f
		fi
	done
	exit 1
fi

for x in $@; do
	if test -f $path/shaders/$x; then
		compile_shader $path/shaders/$x
	fi
done
