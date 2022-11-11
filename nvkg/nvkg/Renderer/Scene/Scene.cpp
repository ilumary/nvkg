#include <nvkg/Renderer/Scene/Scene.hpp>

namespace nvkg {

    Scene::Scene(std::string name) : component(name) {}

    Scene::~Scene() {}
            
    void Scene::add_pointlight(PointLight* pointlight, uint16_t count) {
        for(uint16_t i = 0; i < count; ++i) {
            pointlights.push_back(&pointlight[i]);
        }
        updated = true;
    }    
}
