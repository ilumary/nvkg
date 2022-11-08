ifneq ("$(wildcard .env)","")
    include .env
endif

# Set debugging build flags
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	override CXXFLAGS += -g -DDEBUG
else
    override CXXFLAGS += -DNDEBUG
endif

# Set validation layer build flags
ifeq ($(ENABLE_VALIDATION_LAYERS), 1)
    PACKAGE_FLAGS := --include-validation-layers
endif

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
platformpth = $(subst /,$(PATHSEP),$1)

libDir := $(abspath lib)
buildDir := $(abspath bin)
externDir := $(abspath extern)
assetsDir := $(abspath assets)
scriptsDir := $(abspath scripts)
outputDir := $(abspath output)
executable := app
target := $(buildDir)/$(executable)
sources := $(call rwildcard,nvkg/,*.cpp)
objects := $(patsubst nvkg/%, $(buildDir)/%, $(patsubst %.cpp, %.o, $(sources)))
depends := $(patsubst %.o, %.d, $(objects))

includes = -I $(abspath nvkg) -I $(externDir)/vulkan/include -I $(externDir)/glfw/include -I $(externDir)/glm -I $(externDir)/tinyobjloader -I $(externDir)/stb -I $(externDir)/vulkan/SPIRV-Cross/
linkFlags = -L $(libDir) -lglfw3 -L $(libDir) -lspirv-cross -L/opt/homebrew/opt/gcc/lib/gcc/12/
compileFlags := -std=c++2b $(includes)
glfwLib := $(libDir)/libglfw3.a

vertSources = $(call rwildcard,shaders/,*.vert)
vertObjFiles = $(patsubst %.vert,$(buildDir)/%.vert.spv,$(vertSources))
fragSources = $(call rwildcard,shaders/,*.frag)
fragObjFiles = $(patsubst %.frag,$(buildDir)/%.frag.spv,$(fragSources))

UNAMEOS := $(shell uname)
ifeq ($(UNAMEOS), Linux)

    platform := linux
    CXX ?= g++
    libSuffix = so
    volkDefines = VK_USE_PLATFORM_XLIB_KHR
    linkFlags += -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -no-pie
endif
ifeq ($(UNAMEOS),Darwin)
    
    platform := macos
    #CXX = /opt/homebrew/opt/llvm/bin/clang++
	CXX = aarch64-apple-darwin22-c++-12
    volkDefines = VK_USE_PLATFORM_MACOS_MVK
    linkFlags += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT
endif

PATHSEP := /
MKDIR := mkdir -p
RM := rm -rf
COPY = cp -r $1$(PATHSEP)$3 $2
COPY_DIR = $(call COPY,$1,$2,$3)

glslangValidator := $(externDir)/glslang/build/install/bin/glslangValidator
packageScript := $(scriptsDir)/package.sh

# Lists phony targets for Makefile
.PHONY: all app release clean

all: app release clean 

app: $(target)

# Link the program and create the executable
$(target): $(objects) $(glfwLib) $(vertObjFiles) $(fragObjFiles) $(buildDir)/lib $(buildDir)/assets
	$(CXX) $(objects) -o $(target) $(linkFlags)

$(buildDir)/%.spv: % 
	$(MKDIR) $(call platformpth, $(@D))
	$(glslangValidator) $< -V -o $@

$(glfwLib):
	$(MKDIR) $(call platformpth,$(libDir))
	$(call COPY,$(call platformpth,$(externDir)/glfw/src),$(libDir),libglfw3.a)

$(buildDir)/lib:
	$(MKDIR) $(call platformpth,$(buildDir)/lib)
	$(call COPY_DIR,$(call platformpth,$(externDir)/vulkan/lib),$(call platformpth,$(buildDir)/lib))

$(buildDir)/assets:
	$(MKDIR) $(call platformpth,$(buildDir)/assets)
	$(call COPY_DIR,$(call platformpth,$(assetsDir)),$(call platformpth,$(buildDir)/assets))

# Add all rules from dependency files
-include $(depends)

# Compile objects to the build directory
$(buildDir)/%.o: nvkg/%.cpp Makefile
	$(MKDIR) $(call platformpth,$(@D))
	$(CXX) -MMD -MP -c $(compileFlags) $< -o $@ $(CXXFLAGS) -D$(volkDefines)

package: app
	$(packageScript) "nvkg" $(outputDir) $(buildDir) $(PACKAGE_FLAGS)

clean: 
	$(RM) $(call platformpth, $(buildDir))
	$(RM) $(call platformpth, $(outputDir))
