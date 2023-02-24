#ifndef NVKG_RENDERER_HPP
#define NVKG_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Components/component.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>

namespace nvkg {

    class Renderer {

        public:

            Renderer();
            ~Renderer();

            void render(VkCommandBuffer& command_buffer, std::shared_ptr<CameraNew> camera, ecs::registry& registry);

            void recreate_materials();

        private:
            //TODO this needs to change
            //Model light_model;
            //std::unique_ptr<Material> light_material;
            //std::vector<glm::vec2> point_light_vertices = std::vector<glm::vec2>(0);
            //std::vector<uint32_t> point_light_indices = std::vector<uint32_t>(0);
    };
}

#endif
