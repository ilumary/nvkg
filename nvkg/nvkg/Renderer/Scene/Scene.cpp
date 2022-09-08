#include <nvkg/Renderer/Scene/Scene.hpp>

namespace nvkg {

    Scene::Scene(std::string name) {
        identifier = INTERN_STR(name.c_str());
        shapes_3d = std::vector<Components::Shape>();
    }

    void Scene::update() {
        pointlights.at(2).position.x += .01f;
        updated = true;

        //TODO make pointers from main to scene to update components from main
        // figure out how to check for variable updated
        // maybe add map with id for each game object to update
    }

    void Scene::add_shape_3d(Components::Shape* shapes, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            shapes_3d.push_back(shapes[i]);
        }
        updated = true;
    }

    void Scene::add_pointlight(LightRenderer::PointLightData* pointlight, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            pointlights.push_back(pointlight[i]);
        }
        updated = true;
    }

    Components::Shape* Scene::get_3d_shapes(uint16_t& count) {
        count = shapes_3d.size();
        return shapes_3d.data();
    }

    LightRenderer::PointLightData* Scene::get_pointlights(uint16_t& count) {
        count = pointlights.size();
        return pointlights.data();
    }
}