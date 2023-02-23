#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Shader/Shader.hpp>
#include <nvkg/Renderer/DescriptorPool/DescriptorPool.hpp>
#include <nvkg/Renderer/Texture/TextureManager.hpp>

#include <vector>
#include <array>
#include <map>
#include <chrono>
#include <thread>
#include <functional>
#include <algorithm>

namespace nvkg {

    //TODO get this data from physical device
    #define MAX_DESCRIPTOR_SETS 8
    #define MAX_DESCRIPTOR_BINDINGS_PER_SET 4

    #define VERTEX_BUFFER_BIND_ID 0
    #define INSTANCE_BUFFER_BIND_ID 1

    

    class Material {
        public:

            struct instance_binding_data {
                bool instancing_enabled = false;
                uint32_t per_vertex_size = 0;
                uint32_t per_instance_size = 0;
            };
            
            //TODO somehow get pointer to global ubo for continuos update without explicitly calling update from renderer
            struct MaterialConfig {
                std::vector<std::string> shaders;
                std::map<std::string, SampledTexture*> textures;
                std::function<void(PipelineInit& pipeline)> pipeline_configurator = {};
                instance_binding_data instance_data{};
            } config_;

            Material(const MaterialConfig config);

            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;

            ~Material();

            void set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data);
            void set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data);

            void push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data);
            
            void bind(VkCommandBuffer commandBuffer);

        private:

            ShaderResource& get_res(Utils::StringId id);
            bool has_res(Utils::StringId id);
            void set_texture(SampledTexture* tex, std::string tex_name);

            void create_material();

            void set_shader_props(const std::unique_ptr<ShaderModule>& shader, uint64_t& offset, uint16_t& res_counter);
            void prepare_desc_set_layouts();
            void prepare_pipeline();
            void setup_descriptor_sets();

            std::map<VkShaderStageFlagBits, std::unique_ptr<ShaderModule>> shaders;

            Buffer::Buffer buffer { VK_NULL_HANDLE };
            uint64_t buffer_size = 0;

            /*  
            * Holds all shader resources as internal properties
            * First 8 bits are continuous index, last 8 bits are actual descriptor set
            * This is needed if one descriptor set is left empty, i.e. descriptor sets 0,1 and 3 are used, but 2 is not
            */
            std::map<uint16_t, std::vector<ShaderResource>> resources_per_set{};

            const unsigned short set_mask = 0x00FF;
            const unsigned short index_mask = 0xFF00;

            std::map<uint32_t, SampledTexture*> textures{};

            std::map<std::string, VkPushConstantRange> push_constants{};

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
            std::vector<VkDescriptorSet> descriptor_sets{};
            
            Pipeline pipeline;
            VkPipelineLayout pipeline_layout {VK_NULL_HANDLE};
    };
}
