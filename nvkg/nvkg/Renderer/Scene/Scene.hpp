#ifndef NVKG_SCENE_HPP
#define NVKG_SCENE_HPP

#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Components/component.hpp>

namespace nvkg {

    class Scene {
        public:

            Scene(std::string name);
            ~Scene();

            void set_camera(Camera* camera) { active_camera = camera; }
            CameraData get_camera_data() { return { active_camera->get_proj(), active_camera->get_view()}; };

        private:

            Camera* active_camera;
    };
}

#endif
