#ifndef NVKG_LIGHT_RENDERER_HPP
#define NVKG_LIGHT_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Components/component.hpp>

#include <span>

namespace nvkg {

    class LightRenderer {
        public:

            struct PointLightShaderData {
                glm::vec4 light_color = glm::vec4(1.f, 1.f, 1.f, 1.f);
                glm::vec4 ambient_color = glm::vec4(1.f, 1.f, 1.f, .02f);
                alignas(16) glm::vec3 position = glm::vec3(0.f);
            };

            struct PointLightPushConstants {
                glm::vec4 position{};
                glm::vec4 color{};
                float radius;
            };

            LightRenderer();
            ~LightRenderer();

            void init(const char* globalDataAttributeName, const uint64_t& globalDataSize);

            void destroy();

            void render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData, std::span<PointLight*> pnt_lgts);

            void recreate_materials();

        private:

            struct PointLightVertex {
                alignas(16) glm::vec2 position;
            };

            Model light_model;
            std::unique_ptr<Material> light_material;

            Utils::StringId glob_data_id;

            std::vector<glm::vec2> point_light_vertices = std::vector<glm::vec2>(0);
            std::vector<uint32_t> point_light_indices = std::vector<uint32_t>(0);
    };
}

#endif
