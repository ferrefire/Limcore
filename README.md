# Limcore

### A modern C++ framework for building Vulkan applications with less boilerplate and full low-level control.

Limcore is a C++ graphics library created to simplify and accelerate development with the [Vulkan graphics API](https://www.vulkan.org/).

Vulkan provides extensive control over modern GPU hardware, however that control comes with significant setup complexity and repeated resource-management code. Limcore reduces this overhead through configurable, object-oriented abstractions for Vulkan resources, rendering operations, asset loading, mathematics, input, timing, and application management.

The library provides sensible defaults for fast development while preserving direct access to the underlying Vulkan resources whenever low-level control is required.

## Table of Contents
- [Focus](#focus)
- [Features](#features)
- [Building](#building)
- [Documentation](#documentation)
- [License](#license)

## Focus

Limcore was designed around five primary goals:
### 1. Reduce Vulkan boilerplate
   Repetitive resource creation and configuration should not dominate application code.
### 2. Provide useful defaults
   Common resources should be easy to create without requiring a large amount of configuration.
### 3. Preserve full control
   Abstraction should not prevent developers from accessing native Vulkan resources or configuring advanced behaviour.
### 4. Streamline application pipeline
   A clear and simple structure should be used to keep an application organized and readable.
### 5. Support complete applications
   Rendering, mathematics, assets, input, timing, debugging, and application management should work together as a unified library.

## Features

### Vulkan resource abstractions

Limcore provides C++ handle classes for the major resources used by a Vulkan application, including:
- Buffers and memory
- Graphics and compute pipelines
- Render passes and framebuffers
- Command pools and command buffers
- Physical and logical devices
- Descriptor sets, layouts, and pools
- Swapchains and window surfaces
- Images, image views, and samplers
- Synchronization primitives

Resources can be created using default configurations or customized through explicit configuration structures.

Although these classes remove much of Vulkan's repetitive setup code, they do not hide the underlying API. Native Vulkan handles and resources remain accessible if more advanced behaviour is needed.

### Rendering system

The renderer manages common rendering and compute operations, including:
- Render-pass execution
- Graphics draw commands
- Compute dispatch commands
- Command recording and submission
- Per-frame rendering operations

This provides a structured rendering workflow while keeping synchronisation and resource transitions explicit.

### Hardware and device selection

Limcore includes utilities for querying and selecting an appropriate rendering device.

These utilities can inspect and compare:
- Available GPUs
- Queue-family support
- Window-surface compatibility
- Swapchain support
- Required device extension
- Vulkan features and capabilities
- Device suitability and ranking

This helps applications select the most appropriate physical device instead of relying on a fixed adapter.

### Mathematics

The library contains custom vector and matrix mathematics classes with functionality comparable to commonly used graphics math libraries such as GLM.

The math system is fully templated and supports:
- Vectors of multiple dimensions
- Matrices of multiple sizes
- Common arithmetic operations
- Vector and matrix transformations
- Graphics-related projection and view calculations
- Multiple underlying numeric types

### Camera system

The camera class provides reusable camera behaviour for real-time 3D applications, including:
- Position and orientation management
- Translation and rotation
- View-matrix generation
- Perspective configuration
- Input-driven camera movement

### Asset loading

Limcore includes asset-loading functionality for textures and 3D models.

The image-loading system supports JPEG images and includes utilities for:
- Texture loading of different color formats
- Mipmap generation
- GPU texture preparation
- Compression to BC texture formats

The model-loading system supports:
- Wavefront OBJ models
- glTF models
- Mesh and vertex data extraction

### Mesh utilities

Limcore provides fully templated classes for mesh management and real-time procedural shape generation.

Mesh-related functionality includes:
- Mesh resource management
- Vertex and index configurability and handling
- GPU buffer preparation
- Shape integration

Shape generation includes:
- Generation of numerous shape primitives
- Shape combining and blending
- Configurable vertex and index types
- Normal vector generation
- Translation and rotation

These utilities make it possible to work with both imported assets and meshes generated at runtime. It is also viable to modify imported meshes with the use of the procedural shape system.

### Input and timing

Limcore includes usefull systems for managing input and timing utility. These systems efficiently retrieve and process data from the [GLFW](https://github.com/glfw/glfw) library.

The input system includes functionality such as:
- Keyboard input querying
- Mouse input querying
- Only tracking used input
- Per-frame input information

The timing system allows for:
- Delta time calculation
- Average and current frames per second
- Exposed timing ticks
- Timers
- Application runtime tracking

### User interface

[ImGui](https://github.com/ocornut/imgui) is used for application UI creation. Limcore provides an easy to use system that acts as a layer on top of [ImGui](https://github.com/ocornut/imgui). It focuses on start-only UI creation which is then handled separately. This removes cluttered UI code from per-frame update calls and moves them to application start calls.

The UI system allows for:
- One-time interface declarations
- Component nesting
- Function registering to UI components
- Several interface and data types

### Application management

A central manager acts as the core of a Limcore application, creating and initiating all major resources. It can be configured to alter its behaviour to applications needs.

Systems and callbacks can be registered for different stages of the application lifecycle:
- Startup
- Per-frame updates
- Pre-frame updates
- Post-frame updates
- Shutdown

Most Limcore systems are tightly linked to and managed by this manager. This way everything can be properly tracked and cleanly shut down or fixed in case of a problem. It provides a consistent structure for initializing, running, and cleaning up application components.

### Debugging

The library includes additional debugging functionality by processing custom print statements at compile time. This allows for the printing of enum values and bit flag names.

### Project tools

Limcore provides scripts for project and shader management. Depending on the target platform, these scripts can automate tasks such as:
- Fetching dependencies
- Configuring the project
- Building the library
- Compiling the application
- Compiling GLSL shaders to SPIR-V
- Cleaning up files

## Building

### Requirements

For Linux:
- [CMake](https://cmake.org/) version 3.22 or newer

For Windows:
- [Visual Studio](https://visualstudio.microsoft.com/) 2022 or newer
- [CMake](https://cmake.org/) version 3.22 or newer

### Dependencies

Limcore was build with as few dependencies as possible, making it easy to integrate instead of a chore. All the dependencies it does have, are automatically fetched and managed by Limcore itself.

A list of external libraries used in Limcore:
- [GLFW](https://github.com/glfw/glfw), used for window creation, surface integration, and input
- [ImGui](https://github.com/ocornut/imgui), used for graphical user interfaces

### Project integration

To build Limcore into your project, you should make use of CMake's FetchContent functionality. Simply add the following instructions into your `CMakeLists.txt` file and you're set:
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

## Documentation

Important note: The Limcore documentation is outdated and incomplete. It should only be consulted as a general reference and will be updated soon.

The Limcore source code is documented using Doxygen. The documentation contains class descriptions, API references, configuration options, and information about the relationships between Limcore's systems. A PDF can be downloaded [here](https://github.com/user-attachments/files/29220679/refman.pdf).

### Example application

The [Terrain](https://github.com/ferrefire/Terrain) project is a complex project made with Limcore. It should cover most of Limcore's functionality.

## License

This project is licensed under the MIT License, see LICENSE.txt for more information.

Created by Ferre Molenbeek.
