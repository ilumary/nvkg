#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>
#include <nvkg/Renderer/Lights/PointLight.hpp>
#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Renderer/Renderers/Renderer3D/LightRenderer.hpp>
#include <nvkg/Renderer/Renderers/Renderer3D/ModelRenderer.hpp>

namespace nvkg {

    class Renderer3D {

        #define MAX_LIGHTS 10

        public:

            struct GlobalData {
                CameraData cameraData;
                PointLight::Data lightData[10];
                int light_index;
            };

            static void init();

            static void draw_model(Model* model, const glm::vec3& position, const glm::vec3& scale = glm::vec3{1.f}, const glm::vec3& rotation = glm::vec3{0.f});
            static void draw_billboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& colour);
            static void draw_line(const glm::vec3& origin, const glm::vec3& destination, const glm::vec3& colour);
            static void draw_rect(const glm::vec3& position, const glm::vec2& scale, glm::vec3 color);
            static void add_point_light(const glm::vec3& position, const float& radius, const glm::vec4& colour, const glm::vec4& ambientColor);

            static void recreate_materials();

            static void render(VkCommandBuffer& commandBuffer, const CameraData& globalData);
            static void flush();

            static void destroy();

        private:
        
            static Utils::StringId globalDataId;

            static ModelRenderer modelRenderer;
            static LightRenderer lightRenderer;

            static GlobalData global3DData;
    };
}