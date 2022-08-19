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

    class ShaderModule {
        public:

            struct VertexBinding {
                std::vector<VertexDescription::Attribute> attributes{};
                uint32_t vertexStride = 0;
            };

            struct Uniform {
                Utils::StringId id;
                uint32_t binding = 0; 
                uint64_t size = 0;
                size_t arraySize = 0;
                size_t dyn_count = 1;
                VkDescriptorType type;
            };

            VkShaderModule shader_module{};
            VkShaderStageFlagBits shader_stage{};
            std::string entrance_function;

            // specifies the binding order of the model descriptor.
            std::vector<Utils::Descriptor::DescriptorInfo> material_descriptor_orderings{};
            std::vector<VkPushConstantRange> push_constants{};
            std::vector<VertexBinding> vertex_bindings{};
            std::vector<Uniform> uniforms{};

            uint32_t combined_uniform_size = 0;

            ShaderModule() = default;
            ~ShaderModule() = default;
            ShaderModule(const ShaderModule&) = default;
            ShaderModule& operator=(const ShaderModule&) = default;
            
            void create(std::string name, std::string stage, std::string entrance_function = "main");

            void cleanup();

        private:
            VulkanDevice *device;

            std::string filename;
            std::string filepath;
            std::vector<char> binary_data;

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

            /*
            * Parses binary data and returns it as an array of chars.
            */
            std::vector<char> loadSpirVBinary(std::string file_name);

            /*
            * Uses SPIRV-Cross to perform runtime reflection of the spriv shader to analyze descriptor binding info.
            */
            void reflect_descriptor_types(std::vector<uint32_t> spirv_binary, VkShaderStageFlagBits shader_stage);
    };
}