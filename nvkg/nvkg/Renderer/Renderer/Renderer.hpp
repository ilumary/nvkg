#ifndef NVKG_RENDERER_HPP
#define NVKG_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Scene/Scene.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>

namespace nvkg {

    class Renderer {

        public:

            struct GlobalData {
                CameraData cameraData;
                point_light light_data[10];
                int light_index;
            };

            Renderer();
            ~Renderer();

            void render(VkCommandBuffer& command_buffer, Scene* active_scene, const ecs::registry& registry);

            void recreate_materials();

            void destroy();

        private:

            Utils::StringId global_data_id, transform_id;

            void update_global_ubo(Scene* scene, GlobalData* global_3d_data);

            //TODO this needs to change
            Model light_model;
            std::unique_ptr<Material> light_material;
            std::vector<glm::vec2> point_light_vertices = std::vector<glm::vec2>(0);
            std::vector<uint32_t> point_light_indices = std::vector<uint32_t>(0);
    };
}

#endif
