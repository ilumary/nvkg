# NVKG

NVKG is a rendering engine and my next attempt to finally master vulkan (and modern c++). 

Currently (and for the forseeable future) only linux and macos are supported. I simply don't have access to a windows machine.

Any bugs or problems can be reported via the repo's issues.

The dependencies are reduced to a minimum as the majority of external code is included through git submodules.

Current highlights are:
  - Automated descriptor creation using SPIRV-Cross reflection
  - Material based rendering
  - Fully functional Entity Component System
  - Font rendering using signed distance fields

Next up on my to-do list will be:
  1. Improvements to renderer (instanced and indirect rendering, culling, lod, multithreading)
  2. UI Framwork (i.e. buttons, ...)
  3. Proper asset manager (unified way to load all kinds of files)
  4. Audio support
  5. Physics support
  6. Serialisation
  7. And everything that comes up inbetween...

## Contents

- [NVKG](#nvkg)
  - [Contents](#contents)
  - [Getting Started](#getting-started)
    - [Dependencies](#dependencies)
    - [Setup](#setup)
    - [Building and execution](#building-and-execution)
    - [Modifications \& Contributions](#modifications--contributions)
  - [Contributors](#contributors)
  - [References](#references)

## Getting Started

### Dependencies

The following are dependencies for building and running NVKG:

- [Cmake](https://cmake.org/) (linux & macos)
- [GNU Make](https://www.gnu.org/software/make/) (linux & macos)
- [Python](https://www.python.org/) (linux & macos)

All other libraries are within git submodules.

### Setup

Execute the provided script ```scripts/setup.sh```. If you wish to build with vulkan validation layers, execute with ```--include-validation-layers```. No more action should be needed. 

Linux Users may need to download the following libs: ```libx11-xcb-dev libxkbcommon-dev libwayland-dev libxrandr-dev libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev```.

### Building and execution

Build with ```make app``` and execute with ```./bin/app```.

### Modifications & Contributions

If you want to modify anything, the ```compile_commands.json``` for the clangd language server can be created using [bear](https://github.com/rizsotto/Bear).

## Contributors

- [ilumary](https://github.com/ilumary) - Christoph Britsch (Creator)

## References

- [canonical project structure](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html)
- [sascha willem's examples](https://github.com/SaschaWillems/Vulkan)
- [using spirv-cross with vulkan](https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/4-Using-spir-v-with-spirv-cross.pdf)
