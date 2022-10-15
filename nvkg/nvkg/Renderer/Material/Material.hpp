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
            
            enum PolygonMode {
                FILL = 0, 
                LINE = 1, 
                POINT = 2
            };

            enum Topology {
                LINE_LIST = 1,
                LINE_STRIP = 2,
                TRIANGLE_LIST = 3
            };

            NVKGMaterial();
            NVKGMaterial(ShaderModule* vertex_shader, ShaderModule* fragment_shader);

            NVKGMaterial(const NVKGMaterial&) = delete;
            NVKGMaterial& operator=(const NVKGMaterial&) = delete;

            ~NVKGMaterial();

            void set_polygon_mode(PolygonMode mode) { shader_config.mode = mode; }
            void set_topology(Topology topology) { shader_config.topology = topology; }
            void create_material();

            void set_uniform_data(VkDeviceSize dataSize, const void* data);
            void set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data);
            void set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data);

            void set_vert_shader_new(ShaderModule* shader);
            void set_frag_shader_new(ShaderModule* shader);

            bool has_prop(Utils::StringId id);

            void push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data);
            void set_texture(SampledTexture* tex, std::string tex_name, VkShaderStageFlagBits shader_stage);

            void bind(VkCommandBuffer commandBuffer);

            void recreate_pipeline();
            void prepare_desc_set_layouts();
            void prepare_pipeline();
            void setup_descriptor_sets();
            void destroy_material();

            static void create_materials(std::initializer_list<NVKGMaterial*> materials);

        private:

            struct MaterialTexture {
                Utils::StringId id;
                VkShaderStageFlagBits stages;
                SampledTexture* texture = nullptr;
            };

            ShaderResource& get_res(Utils::StringId id);

            void add_shader(ShaderModule* shader);
            void set_shader_props(ShaderModule* shader, uint64_t& offset, uint16_t& res_counter);

            void create_layout(
                VkDescriptorSetLayout* layouts = nullptr, 
                uint32_t layoutCount = 0, 
                VkPushConstantRange* pushConstants = nullptr, 
                uint32_t pushConstantCount = 0
            );

            struct {
                PolygonMode mode = PolygonMode::FILL;
                Topology topology = Topology::TRIANGLE_LIST;
            } shader_config;

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

            std::map<uint32_t, MaterialTexture> textures{};

            std::map<std::string, VkPushConstantRange> push_constants_new{}; //
            std::vector<VertexDescription::Binding> vertex_binds{};

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
            std::vector<VkDescriptorSet> descriptor_sets{};
            
            Pipeline pipeline;
            VkPipelineLayout pipeline_layout {VK_NULL_HANDLE};

            bool isFreed = false;
    };
}
