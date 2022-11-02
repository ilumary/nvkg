#ifndef NVKG_SCENE_HPP
#define NVKG_SCENE_HPP

#include <nvkg/Components/Shape.hpp>
#include <nvkg/Renderer/Renderer/Renderer3D/LightRenderer.hpp>
#include <nvkg/Renderer/Renderer/Renderer2D/ui_renderer.hpp>
#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Components/component.hpp>

namespace nvkg {

    class Scene : public component<Scene> {
        public:

            Scene(std::string name);
            ~Scene();

            bool _on_load() { return true; }
            bool _on_delete() { return true; }

            bool updated;

            void add_shape_3d(Components::Shape* shapes, uint16_t count = 1);
            void add_pointlight(PointLight* pointlight, uint16_t count = 1);
            void add_ui_component(UIComponent* ui_component, uint16_t count = 1);

            inline std::span<Components::Shape*> get_3d_shapes() { return shapes_3d; } 
            inline std::span<PointLight*> get_pointlights() { return pointlights; }
            inline std::span<UIComponent*> get_ui_components() { return ui_components; }

            void set_camera(Camera* camera) { active_camera = camera; }
            CameraData get_camera_data() { return { active_camera->get_proj(), active_camera->get_view()}; };

        private:

            Camera* active_camera;

            std::vector<Components::Shape*> shapes_3d{};
            std::vector<PointLight*> pointlights{};
            std::vector<UIComponent*> ui_components{};
    };

}

#endif
