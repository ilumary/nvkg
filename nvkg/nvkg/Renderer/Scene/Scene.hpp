#pragma once

#include <nvkg/Components/Shape.hpp>
#include <nvkg/Renderer/Renderer/Renderer.hpp>

namespace nvkg {

    struct Billboard {
        glm::vec3 position;
        glm::vec2 scale;
        glm::vec4 colour;
    };

    struct PointLightInit {
        glm::vec3 position;
        float radius;
        glm::vec4 colour; 
        glm::vec4 ambientColor;
    };

    struct Line {
        glm::vec3 origin;
        glm::vec3 destination;
        glm::vec3 colour;
    };

    class Scene {
        public:

            Scene(std::string name);
            ~Scene();

            std::string identifier;

            void draw();
            void update();

            void add_shape_3d(Components::Shape* shapes, uint16_t count = 1);
            void add_shape_2d(Components::Shape* shapes, uint16_t count = 1);
            void add_billboard(Billboard* billboard, uint16_t count = 1);
            void add_pointlight(PointLightInit* pointlight, uint16_t count = 1);
            void add_line(Line* line, uint16_t count = 1);

            void set_camera(Camera* camera) { active_camera = camera; }
            CameraData get_camera_data() { return { active_camera->get_proj(), active_camera->get_view()}; };

        private:

            Camera* active_camera;

            std::vector<Components::Shape> shapes3d;
            std::vector<Components::Shape> shapes2d;
            std::vector<Billboard> billboards;
            std::vector<Line> lines;

    };

}