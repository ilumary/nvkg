#ifndef NVKG_RENDERER_HPP
#define NVKG_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Scene/Scene.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Renderer/Renderer3D/LightRenderer.hpp>
#include <nvkg/Renderer/Renderer/Renderer2D/ui_renderer.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>

namespace nvkg {

    class Renderer {

        public:

            struct GlobalData {
                CameraData cameraData;
                LightRenderer::PointLightShaderData lightData[10];
                int light_index;
            };

            Renderer();
            ~Renderer();

            void render(VkCommandBuffer& command_buffer, Scene* active_scene, const ecs::registry& registry);

            void recreate_materials();

            void destroy();

        private:

            Utils::StringId global_data_id, transform_id;

            LightRenderer* light_renderer;

            void update_global_ubo(Scene* scene, GlobalData* global_3d_data);
    };
}

#endif
