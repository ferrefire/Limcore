# Limcore

Limcore is a lightweight platform independent graphics library that uses Vulkan and GLFW. Limcore does not use any other external libraries. All the dependencies are automatically downloaded and compiled alongside Limcore.

## Table of Contents
- [Installation](#installation)
- [Setup](#setup)
- [Usage](#usage)

## Installation
### Dependencies
For Linux:
- CMake version 3.22 or newer.

For Windows:
- A bash terminal: something like 'git bash'.
- Visual studio 2022 or newer.
- CMake version 3.22 or newer.

### CMake
To build Limcore into your project using CMake, you can use CMake's FetchContent functionality.
Simply add the following into your CMakeLists.txt file and your set:
```cmake
include(FetchContent) # If not already included

FetchContent_Declare(
    limcore
    GIT_REPOSITORY https://github.com/ferrefire/Limcore.git
    GIT_TAG main
	GIT_SHALLOW 1 # To reduce project size
)

FetchContent_MakeAvailable("limcore")

target_link_libraries(${PROJECT_NAME} PUBLIC "limcore")
```
Limcore handles all the dependencies and is platform independent. So it should "just work".

## Setup
Limcore also provides a quick and easy way to create and setup a new project. All you need to do is download the 
![new-project.sh](https://github.com/ferrefire/Limcore/blob/main/new-project.sh) script and run it.

It will automatically create the source file directories needed for Limcore. It will also create a CMakeLists file and add glslang to the project for shader compiling. To create a Limcore project with the new-project.sh script, you can run the following command:
```bash
./new-project.sh # To create and setup everything

./new-project.sh d # To create and setup specific elements

./new-project.sh -override # To add specific behaviour by using options
```

### Creation commands
- d (to create all the source directories).
- s (to download and initiate the setup script).
- c (to download and initiate the CMake file).
- g (to download and initiate the shader compiling scripts).

### Creation options
- -override or -o (to overwrite all the helper scripts and CMake files regardless of if they already exist).
- -clean or -c (to clean up and remove downloaded files).

The new-project.sh script will also add two very usefull scripts for managing the project (building, compiling, running, managing dependencies, etc.)

### Setup.sh
The setup.sh script is very usefull to quickly manage your Limcore project. It's usage is simple and straightfoward.

To build the CMake files and fetch or update any dependencies, execute the following command:
```bash
./setup.sh build # Or 'bd'
```

To compile the project into an executable, execute the following command:
```bash
./setup.sh compile # Or 'cmp'
```

To run the project, execute the following command:
```bash
./setup.sh run # Or 'r'
```
And to add application arguments, you can add them after the "run" command by using a '-':
```bash
./setup.sh run -fs # To run the application in fullscreen
```

To clean the project and remove any downloaded dependencies, execute the following command:
```bash
./setup.sh clean # Or 'cl'
```

### Shader-Compiler.sh
The shader-compiler.sh script is a swift way to compile your shaders from GLSL into SPIRV code. Simply put your shaders (.vert, .frag, .comp, etc.) in the "shaders" folder and execute the following command:
```bash
./shader-compiler all # Or example.vert example.frag for compiling specifig shaders
```

## Usage
test