cmake_minimum_required(VERSION 3.22)
project(limcore VERSION 1.0)

option(DEV "DEV" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FILE(GLOB SOURCE_FILES sources/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
#add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC "includes")

set(GLFW_BUILD_WAYLAND False)#check false replace by off
set(BUILD_WSI_WAYLAND_SUPPORT False)
set(ENABLE_OPT False)
set(UPDATE_DEPS OFF)
set(BUILD_TESTS OFF)

execute_process(COMMAND pwd)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("glfw")
target_link_libraries(${PROJECT_NAME} PUBLIC "glfw")

FetchContent_Declare(
    Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG 595c8d4794410a4e64b98dc58d27c0310d7ea2fd
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("Vulkan-Headers")
target_link_libraries(${PROJECT_NAME} PUBLIC "Vulkan-Headers")

FetchContent_Declare(
    Vulkan-Loader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG faeb5882c7faf3e683ebb1d9d7dbf9bc337b8fa6
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("Vulkan-Loader")
target_link_libraries(${PROJECT_NAME} PUBLIC "vulkan")

if(DEV)
	FetchContent_Declare(
	    glslang
	    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
	    GIT_TAG fa9c3deb49e035a8abcabe366f26aac010f6cbfb
		GIT_SHALLOW 1
	)
	FetchContent_MakeAvailable("glslang")
endif()