#include <nvkg/Renderer/Scene/Scene.hpp>

namespace nvkg {

    Scene::Scene(std::string name) : Component(name) {}

    Scene::~Scene() {}

    void Scene::add_shape_3d(Components::Shape* shapes, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            shapes_3d.push_back(&shapes[i]);
        }
    }
            
    void Scene::add_pointlight(LightRenderer::PointLight* pointlight, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            pointlights.push_back(&pointlight[i]);
        }
        updated = true;
    }
            
    void Scene::add_ui_component(UIRenderer::UIComponent* ui_component, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            ui_components.push_back(&ui_component[i]);
        }
    }
}
