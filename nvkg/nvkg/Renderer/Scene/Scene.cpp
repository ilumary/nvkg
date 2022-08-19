#include <nvkg/Renderer/Scene/Scene.hpp>

namespace nvkg {

    Scene::Scene(std::string name) {
        identifier = name;
        shapes3d = std::vector<Components::Shape>();
        shapes2d = std::vector<Components::Shape>();
        billboards = std::vector<Billboard>();
        point_lights = std::vector<PointLightInit>();
        lines = std::vector<Line>();
    }

    void Scene::draw() {
        for (auto& shape : shapes3d) {
            nvkg::Renderer3D::draw_model(shape.GetModel(), shape.get_pos(), shape.get_scale(), shape.get_rot());
        }
        /*for(auto& plight : point_lights) {
            nvkg::Renderer3D::add_point_light(plight.position, plight.radius, plight.colour, plight.ambientColor);
        }*/
        for(auto& bboard : billboards) {
            nvkg::Renderer3D::draw_billboard(bboard.position, bboard.scale, bboard.colour);
        }
        for(auto& line : lines) {
            nvkg::Renderer3D::draw_line(line.origin, line.destination, line.colour);
        }
    }

    void Scene::add_shape_3d(Components::Shape* shapes, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            shapes3d.push_back(shapes[i]);
        }
    }

    void Scene::add_shape_2d(Components::Shape* shapes, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            shapes2d.push_back(shapes[i]);
        }
    }

    void Scene::add_billboard(Billboard* billboard, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            billboards.push_back(billboard[i]);
        }
    }

    void Scene::add_pointlight(PointLightInit* pointlight, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            //point_lights.push_back(pointlight[i]);
            nvkg::Renderer3D::add_point_light(pointlight->position, pointlight->radius, pointlight->colour, pointlight->ambientColor);
        }
    }

    void Scene::add_line(Line* line, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            lines.push_back(line[i]);
        }
    }

}