#ifndef NVKG_SCENE_HPP
#define NVKG_SCENE_HPP

#include <nvkg/Components/Shape.hpp>
#include <nvkg/Renderer/Renderer/Renderer3D/LightRenderer.hpp>
#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Components/component.hpp>

namespace nvkg {

    class Scene : public Component {
        public:

            Scene(std::string name);
            virtual ~Scene();

            virtual bool _on_load() { return true; }
            virtual void _on_delete() {}

            Utils::StringId identifier;
            bool updated;

            void add_shape_3d(Components::Shape* shapes, uint16_t count = 1);
            void add_pointlight(LightRenderer::PointLight* pointlight, uint16_t count = 1);

            Components::Shape** get_3d_shapes(uint16_t& count);
            LightRenderer::PointLight** get_pointlights(uint16_t& count);

            void set_camera(Camera* camera) { active_camera = camera; }
            CameraData get_camera_data() { return { active_camera->get_proj(), active_camera->get_view()}; };

        private:

            Camera* active_camera;

            std::vector<Components::Shape*> shapes_3d{};
            std::vector<LightRenderer::PointLight*> pointlights{};
    };

}

#endif
