#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Shader/Shader.hpp>
#include <nvkg/Renderer/DescriptorPool/DescriptorPool.hpp>
#include <nvkg/Renderer/Texture/TextureManager.hpp>

#include <vector>

namespace nvkg {
    class NVKGMaterial {
        public:

        static constexpr size_t MAX_SHADERS = 2;
        static constexpr size_t MAX_BINDINGS_PER_SHADER = 5;
        static constexpr size_t MAX_MATERIAL_BINDINGS = MAX_SHADERS * MAX_BINDINGS_PER_SHADER;
        
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

        void init_descriptors();

        void set_polygon_mode(PolygonMode mode) { shader_config.mode = mode; }
        void set_topology(Topology topology) { shader_config.topology = topology; }
        void create_material();

        void set_uniform_data(VkDeviceSize dataSize, const void* data);
        void set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data);
        void set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data);

        void set_vert_shader_new(ShaderModule* shader);
        void set_frag_shader_new(ShaderModule* shader);

        bool has_prop(Utils::StringId id);

        void push_constant(VkCommandBuffer command_buffer, size_t push_constant_size, const void* data);
        void set_texture(SampledTexture* tex, std::string tex_name, uint32_t binding, VkShaderStageFlagBits shader_stage);

        void bind(VkCommandBuffer commandBuffer);
        void create_pipeline();
        void recreate_pipeline();

        void destroy_material();

        static void create_materials(std::initializer_list<NVKGMaterial*> materials);

        private:

        struct DescriptorBinding {
            VkDescriptorSetLayout layout {VK_NULL_HANDLE};
            VkDescriptorSet descriptorSet {VK_NULL_HANDLE};
            VkDescriptorType type;
        }; 

        struct Property {
            uint32_t binding = 0;
            Utils::StringId id = 0;
            VkShaderStageFlags stage;
            uint64_t offset = 0;
            uint64_t size = 0;
            size_t count = 0;
            DescriptorBinding descriptorBinding;
            SampledTexture* tex = nullptr;
        };

        Property& get_prop(Utils::StringId id);

        void add_shader(ShaderModule* shader);
        void set_shader_props(ShaderModule* shader, uint64_t& offset);

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

        size_t vert_count = 0;
        uint32_t shader_count = 0;

        ShaderModule* vert_shader_new {nullptr};
        ShaderModule* frag_shader_new {nullptr};

        std::vector<Property> prop_vec{};
        std::vector<VkPushConstantRange> push_constants{};

        Buffer::Buffer buffer;
        uint64_t buffer_size = 0;

        std::vector<VkDescriptorSet> desc_sets{};
        std::vector<uint32_t> desc_offs{};

        std::vector<VertexDescription::Binding> vertex_binds{};
        
        Pipeline pipeline;
        VkPipelineLayout pipeline_layout {VK_NULL_HANDLE};

        bool isFreed = false;
        bool is_new_shader_class = false;
    };
}
