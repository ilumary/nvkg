#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Utils/Hash.hpp>
#include <nvkg/Renderer/Utils/Descriptor.hpp>
#include <map>

#include <ostream>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <spirv_glsl.hpp>


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

    class ShaderModule {
        public:

            struct ShaderInit {
                std::string name, stage, entrance_function = "main";
            };

            ShaderModule(ShaderInit init) { create(init.name, init.stage, init.entrance_function); };
            ~ShaderModule() { cleanup(); };

            ShaderModule(const ShaderModule&) = default;
            ShaderModule& operator=(const ShaderModule&) = default;

            struct VertexBinding {
                std::vector<VertexDescription::Attribute> attributes{};
                uint32_t vertexStride = 0;
            };

            VkShaderModule shader_module{};
            VkShaderStageFlagBits shader_stage{};
            std::string entrance_function;

            // specifies the binding order of the model descriptor.
            std::map<std::string, VkPushConstantRange> push_constants_new{}; //
            std::vector<VertexBinding> vertex_bindings{};
            std::vector<ShaderResource> shader_resources{};

        private:
            VulkanDevice *device;

            std::string filename;
            std::string filepath;
            std::vector<char> binary_data;

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

            void create(std::string name, std::string stage, std::string entrance_function = "main");

            void cleanup();

            /*
            * Parses binary data and returns it as an array of chars.
            */
            std::vector<char> loadSpirVBinary(std::string file_name);

            /*
            * Uses SPIRV-Cross to perform runtime reflection of the spriv shader to analyze descriptor binding info.
            */
            void reflect_descriptor_types(std::vector<uint32_t> spirv_binary);

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
