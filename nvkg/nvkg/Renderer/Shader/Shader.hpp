#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Utils/Hash.hpp>
#include <nvkg/Renderer/Utils/Descriptor.hpp>

#include <map>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <iomanip>

#include <spirv_glsl.hpp>

#include "SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/ResourceLimits.h"

namespace nvkg {

    struct ShaderResource {
        Utils::StringId id;
        uint32_t set = 0;
        uint32_t binding = 0;
        uint64_t size = 0;
        size_t arraySize = 0;
        size_t dyn_count = 1;
        uint64_t offset = 0;
        VkDescriptorType type;
        VkShaderStageFlags stage;
    };

    class glsl_runtime_compiler {
        private:

            inline static constexpr TBuiltInResource defaultTBuiltInResource_ = {
                .maxLights =  32,
                .maxClipPlanes =  6,
                .maxTextureUnits =  32,
                .maxTextureCoords =  32,
                .maxVertexAttribs =  64,
                .maxVertexUniformComponents =  4096,
                .maxVaryingFloats =  64,
                .maxVertexTextureImageUnits =  32,
                .maxCombinedTextureImageUnits =  80,
                .maxTextureImageUnits =  32,
                .maxFragmentUniformComponents =  4096,
                .maxDrawBuffers =  32,
                .maxVertexUniformVectors =  128,
                .maxVaryingVectors =  8,
                .maxFragmentUniformVectors =  16,
                .maxVertexOutputVectors =  16,
                .maxFragmentInputVectors =  15,
                .minProgramTexelOffset =  -8,
                .maxProgramTexelOffset =  7,
                .maxClipDistances =  8,
                .maxComputeWorkGroupCountX =  65535,
                .maxComputeWorkGroupCountY =  65535,
                .maxComputeWorkGroupCountZ =  65535,
                .maxComputeWorkGroupSizeX =  1024,
                .maxComputeWorkGroupSizeY =  1024,
                .maxComputeWorkGroupSizeZ =  64,
                .maxComputeUniformComponents =  1024,
                .maxComputeTextureImageUnits =  16,
                .maxComputeImageUniforms =  8,
                .maxComputeAtomicCounters =  8,
                .maxComputeAtomicCounterBuffers =  1,
                .maxVaryingComponents =  60,
                .maxVertexOutputComponents =  64,
                .maxGeometryInputComponents =  64,
                .maxGeometryOutputComponents =  128,
                .maxFragmentInputComponents =  128,
                .maxImageUnits =  8,
                .maxCombinedImageUnitsAndFragmentOutputs =  8,
                .maxCombinedShaderOutputResources =  8,
                .maxImageSamples =  0,
                .maxVertexImageUniforms =  0,
                .maxTessControlImageUniforms =  0,
                .maxTessEvaluationImageUniforms =  0,
                .maxGeometryImageUniforms =  0,
                .maxFragmentImageUniforms =  8,
                .maxCombinedImageUniforms =  8,
                .maxGeometryTextureImageUnits =  16,
                .maxGeometryOutputVertices =  256,
                .maxGeometryTotalOutputComponents =  1024,
                .maxGeometryUniformComponents =  1024,
                .maxGeometryVaryingComponents =  64,
                .maxTessControlInputComponents =  128,
                .maxTessControlOutputComponents =  128,
                .maxTessControlTextureImageUnits =  16,
                .maxTessControlUniformComponents =  1024,
                .maxTessControlTotalOutputComponents =  4096,
                .maxTessEvaluationInputComponents =  128,
                .maxTessEvaluationOutputComponents =  128,
                .maxTessEvaluationTextureImageUnits =  16,
                .maxTessEvaluationUniformComponents =  1024,
                .maxTessPatchComponents =  120,
                .maxPatchVertices =  32,
                .maxTessGenLevel =  64,
                .maxViewports =  16,
                .maxVertexAtomicCounters =  0,
                .maxTessControlAtomicCounters =  0,
                .maxTessEvaluationAtomicCounters =  0,
                .maxGeometryAtomicCounters =  0,
                .maxFragmentAtomicCounters =  8,
                .maxCombinedAtomicCounters =  8,
                .maxAtomicCounterBindings =  1,
                .maxVertexAtomicCounterBuffers =  0,
                .maxTessControlAtomicCounterBuffers =  0,
                .maxTessEvaluationAtomicCounterBuffers =  0,
                .maxGeometryAtomicCounterBuffers =  0,
                .maxFragmentAtomicCounterBuffers =  1,
                .maxCombinedAtomicCounterBuffers =  1,
                .maxAtomicCounterBufferSize =  16384,
                .maxTransformFeedbackBuffers =  4,
                .maxTransformFeedbackInterleavedComponents =  64,
                .maxCullDistances =  8,
                .maxCombinedClipAndCullDistances =  8,
                .maxSamples =  4,
                .maxMeshOutputVerticesNV =  256,
                .maxMeshOutputPrimitivesNV =  512,
                .maxMeshWorkGroupSizeX_NV =  32,
                .maxMeshWorkGroupSizeY_NV =  1,
                .maxMeshWorkGroupSizeZ_NV =  1,
                .maxTaskWorkGroupSizeX_NV =  32,
                .maxTaskWorkGroupSizeY_NV =  1,
                .maxTaskWorkGroupSizeZ_NV =  1,
                .maxMeshViewCountNV =  4,
                .maxDualSourceDrawBuffersEXT =  1,

                .limits =  {
                    .nonInductiveForLoops =  1,
                    .whileLoops =  1,
                    .doWhileLoops =  1,
                    .generalUniformIndexing =  1,
                    .generalAttributeMatrixVectorIndexing =  1,
                    .generalVaryingIndexing =  1,
                    .generalSamplerIndexing =  1,
                    .generalVariableIndexing =  1,
                    .generalConstantMatrixVectorIndexing =  1,
            } };
        
        public:

            struct shader_info {
                std::vector<std::string> compilation_defines_;
                VkShaderStageFlagBits shader_stage_;
                std::string shader_code_;
                std::string entry_;
                bool enable_debug_compilation_ = false;
            };

            static bool preprocess_glsl(const shader_info& info, std::string& glsl_shader_code);
            static bool compile_to_spirv(const shader_info& info, std::vector<uint32_t>& shader_code);
    };

    class ShaderModule {
        public:
            struct file_handle {
                std::filesystem::path path_; //depending on runtime_compilation the path is set
                std::filesystem::file_time_type last_write_time_;
                bool compile_sources = true;
            } file_handle_;

            ShaderModule(std::string file, bool runtime_compilation = true);
            ~ShaderModule();

            ShaderModule(const ShaderModule&) = default;
            ShaderModule& operator=(const ShaderModule&) = default;

            void recompile();

            struct VertexAttributes {
                std::vector<std::pair<uint32_t, VkFormat>> attributes{}; //<offset, format>
                uint32_t vertexStride = 0;
            };

            VkShaderModule shader_module{};
            VkShaderStageFlagBits shader_stage{};

            // specifies the binding order of the model descriptor.
            std::map<std::string, VkPushConstantRange> push_constants_new{};
            VertexAttributes vertex_attributes{};
            std::vector<ShaderResource> shader_resources{};

        private:
            std::vector<char> spirv_bin_data{};
            std::vector<uint32_t> spirv_bin_data_u32{};

            uint32_t combined_uniform_size = 0;

            std::vector<const char *> accepted_material_descriptors = {
                "ambient_map",
                "diffuse_map",
                "specular_map",
                "normal_map",
                "roughness_map",
                "metalness_map",
                "albedo_map",
                "emissiveness_map",
                "ambient_occlusion_map",
                "radiance_map"
            };

            void create();

            /*
            * Parses binary data and returns it as an array of chars.
            */
            std::vector<char> loadSpirVBinary(std::string file_name);

            /*
            * Uses SPIRV-Cross to perform runtime reflection of the spriv shader to analyze descriptor binding info.
            */
            void reflect_descriptor_types(std::vector<uint32_t>& spirv_binary);

            /*
            * Collects all push constant data from shader.
            */
            void reflect_push_constants(spirv_cross::CompilerGLSL &glsl);

            /*
            * Collects all uniform buffer data from shader.
            */
            void reflect_uniform_buffers(spirv_cross::CompilerGLSL &glsl);

            /*
            * Collects all storage buffer data from shader.
            */
            void reflect_storage_buffers(spirv_cross::CompilerGLSL &glsl);

            /*
            * Collects all sampled images from shader.
            */
            void reflect_sampled_images(spirv_cross::CompilerGLSL &glsl);

            /*
            * Collects all vertex shader stage inputs.
            */
            void reflect_stage_inputs(spirv_cross::CompilerGLSL &glsl);
    };
}
