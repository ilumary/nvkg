# NVKG

NVKG is a rendering engine and my next attempt to finally master vulkan (and modern c++). 

Currently (and for the forseeable future) only linux and macos are supported. I simply don't have access to a windows machine.

Any bugs or problems can be reported via the repo's issues.

The dependencies are reduced to a minimum as the majority of external code is included through git submodules.

Current highlights are:
  - Automated descriptor creation using SPIRV-Cross reflection
  - Material based rendering
  - Plug-and-play architecture
  - Model loading
  - Fully functional Entity Component System
  - Signed distance field font rendering

Still to be done are:
  - Compute pipeline builder
  - Examples
  - LUA scripting support
  - UI Framework
  - lots more...

## Contents

- [NVKG](#nvkg)
  - [Contents](#contents)
  - [Getting Started](#getting-started)
    - [Dependencies](#dependencies)
    - [Setup](#setup)
    - [Building and execution](#building-and-execution)
    - [Modifications](#modifications)
  - [Contributors](#contributors)
  - [References](#references)

## Getting Started

### Dependencies

The following are dependencies for building and running NVKG:

- [Cmake](https://cmake.org/) (all platforms)
- [GNU Make](https://www.gnu.org/software/make/) (linux & macos)
- [Python](https://www.python.org/) (all platforms)

All other libraries are within git submodules.

### Setup

Execute the provided script ```scripts/setup.sh```. This should take care of mostly everything.

Also make shure to export your environment variables VULKAN_SDK, VK_ICD_FILENAMES, VK_LAYER_PATH, VK_VERSION.

### Building and execution

Build with ```make app``` and execute with ```./bin/app```.

### Modifications

If you want to modify anything, the ```compile_commands.json``` for the clangd language server can be created using [bear](https://github.com/rizsotto/Bear).

## Contributors

- [ilumary](https://github.com/ilumary) - Christoph Britsch (Creator)

## References

- [canonical project structure](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html)
- [sascha willem's examples](https://github.com/SaschaWillems/Vulkan)
- [using spirv-cross with vulkan](https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/4-Using-spir-v-with-spirv-cross.pdf)
