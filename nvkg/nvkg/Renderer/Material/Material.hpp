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

    struct instance_binding_data {
        bool instancing_enabled = false;
        uint32_t per_vertex_size = 0;
        uint32_t per_instance_size = 0;
    };
    
    //TODO somehow get pointer to global ubo for continuos update without explicitly calling update from renderer
    struct material_config {
        std::vector<std::string> shaders;
        std::map<std::string, SampledTexture*> textures;
        std::function<void(PipelineInit& pipeline)> pipeline_configurator = {};
        instance_binding_data instance_data = {};
    };

    /// @brief forward declare Material
    class Material;

    /// @brief material id used to reference any created material. 32 Bit should be sufficient for enough materials
    using material_id = uint32_t;

    /// @brief material generation used to determine if a material is alive
    using material_generation = uint32_t;

    /// @brief material handle hold an id and a generation 
    struct material_handle {
        public:
            static constexpr auto invalid_id = std::numeric_limits<material_id>::max();
            static constexpr auto invalid_generation = std::numeric_limits<material_generation>::max();

            constexpr material_handle() = default;
            explicit constexpr material_handle(material_id id, material_generation gen = 0) noexcept : m_id_(id), m_gen_(gen) {}

            constexpr auto operator<=>(const material_handle& rsh) const = default;

            [[nodiscard]] constexpr auto id() const noexcept { return m_id_; }
            [[nodiscard]] constexpr auto generation() const noexcept { return m_gen_; }

        private:
            material_id m_id_ {invalid_id};
            material_generation m_gen_ {invalid_generation};
    };

    /// @brief material manager statically manages all materials in order to standardize storing them
    class MaterialManager {
        public:
            /// @brief creates a new material and returns a unique handle
            /// @param mc material config for configuring material
            /// @return material handle
            static const material_handle create(const material_config mc);

            /// @brief retrieves pointer to material
            /// @param mh material handle
            /// @return pointer to material
            static Material* get(const material_handle& mh);

            /// @brief checks if material handle points to valid (='alive') material
            /// @param mh material handle
            /// @return bool
            static const bool alive(const material_handle& mh) noexcept;

            /// @brief invalidates material handle and destructs corresponding material
            /// @param mh material handle
            static const void destroy(const material_handle& mh) noexcept;

            /// @brief destructs all materials. called at program close 
            static const void cleanup() noexcept;

        private:

            static std::vector<material_id> recycled_ids_;
            static std::vector<material_generation> generations_;
            static material_id next_id_;

            static std::vector<std::unique_ptr<Material>> materials_;
    };

    class Material {
        private:

            Material(const material_config config);

            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;

            friend MaterialManager; // make Material only constructible from MaterialManager

        public:

            ~Material();

            void set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data);
            void set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data);

            void push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data);
            
            void bind(VkCommandBuffer commandBuffer);

        protected:

            material_config config_;

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
