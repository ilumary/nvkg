#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Lights/PointLight.hpp>

namespace nvkg {

    class LightRenderer {
        public:

        struct PointLightData {
            glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 0.2f);
            glm::vec4 ambient = glm::vec4(1.f, 1.f, 1.f, .02f);
            glm::vec3 position = glm::vec3(0.f);
            float radius;
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

        void add_point_light(const glm::vec3& position, const float& radius, const glm::vec4& colour, const glm::vec4& ambient);

        void render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData);

        void recreate_materials();

        private: 

        struct PointLightVertex {
            alignas(16) glm::vec2 position;
        };

        Model light_model;
        NVKGMaterial light_material_new;

        Utils::StringId glob_data_id;
        //Utils::StringId light_data_id;

        std::vector<PointLightData> point_lights = std::vector<PointLightData>(0);
        std::vector<glm::vec2> point_light_vertices = std::vector<glm::vec2>(0);
        std::vector<uint32_t> point_light_indices = std::vector<uint32_t>(0);
    };
}