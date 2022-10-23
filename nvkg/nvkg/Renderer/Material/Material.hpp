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

namespace nvkg {

    //TODO get this data from physical device
    #define MAX_DESCRIPTOR_SETS 8
    #define MAX_DESCRIPTOR_BINDINGS_PER_SET 4

    class NVKGMaterial {
        public:

            //TODO somehow get pointer to global ubo for continuos update without explicitly calling update from renderer
            struct MaterialConfig {
                std::vector<ShaderModule::ShaderInit> shaders;
                std::map<std::string, SampledTexture*> textures;
            };

            NVKGMaterial();
            NVKGMaterial(const MaterialConfig config);

            NVKGMaterial(const NVKGMaterial&) = delete;
            NVKGMaterial& operator=(const NVKGMaterial&) = delete;

            ~NVKGMaterial();

            void set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data);
            void set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data);

            bool has_prop(Utils::StringId id);

            void push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data);
            void set_texture(SampledTexture* tex, std::string tex_name);

            void bind(VkCommandBuffer commandBuffer);

            void recreate_pipeline();
            void prepare_desc_set_layouts();
            void prepare_pipeline();
            void setup_descriptor_sets();
            void destroy_material();

        private:

            ShaderResource& get_res(Utils::StringId id);

            void create_material();
            void add_shader(ShaderModule* shader);
            void set_shader_props(ShaderModule* shader, uint64_t& offset, uint16_t& res_counter);

            void create_layout(
                VkDescriptorSetLayout* layouts = nullptr, 
                uint32_t layoutCount = 0, 
                VkPushConstantRange* pushConstants = nullptr, 
                uint32_t pushConstantCount = 0
            );

            uint32_t shader_count = 0;

            ShaderModule* vert_shader {nullptr};
            ShaderModule* frag_shader {nullptr};

            Buffer::Buffer buffer;
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

            std::map<std::string, VkPushConstantRange> push_constants_new{}; //
            std::vector<VertexDescription::Binding> vertex_binds{};

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
            std::vector<VkDescriptorSet> descriptor_sets{};
            
            Pipeline pipeline;
            VkPipelineLayout pipeline_layout {VK_NULL_HANDLE};

            bool isFreed = false;
    };
}
