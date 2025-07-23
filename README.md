# Limcore

![Example Batch](test)

Limcore is a lightweight platform independent graphics library that uses Vulkan and GLFW. Limcore does not use any external libraries (outside of Vulkan and GLFW).

## Table of Contents
- [Installation](#installation)
- [Setup](#setup)
- [Usage](#usage)

## Installation
### CMake
To build Limcore into your project using CMake, you can use CMake's FetchContent functionality.
Simply add the following into your CMakeLists file and your set:
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

## Usage
test