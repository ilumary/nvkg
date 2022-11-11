#include <nvkg/Renderer/Renderer/Renderer.hpp>

namespace nvkg {

    Renderer::Renderer() {
        global_data_id = INTERN_STR("globalData");
        transform_id = INTERN_STR("objectBuffer");

        light_renderer = new LightRenderer();
        light_renderer->init("globalData", sizeof(GlobalData));

        //ui_renderer = new UIRenderer();
        //ui_renderer->init();
    }

    void Renderer::recreate_materials() {
        //model_renderer->recreate_materials();
        light_renderer->recreate_materials();
    }

    void Renderer::destroy() {
        //model_renderer->destroy();
        light_renderer->destroy();
    }

    void Renderer::render(VkCommandBuffer& commandBuffer, Scene* scene, const ecs::registry& registry) {
        GlobalData global_3d_data{};

        update_global_ubo(scene, &global_3d_data); // TODO collect pointlight data another way

        global_3d_data.cameraData = scene->get_camera_data();
        uint64_t globalDataSize = sizeof(global_3d_data);

        light_renderer->render(commandBuffer, globalDataSize, &global_3d_data, scene->get_pointlights());
        //ui_renderer->render(commandBuffer, scene->get_ui_components());

        std::vector<Model::Transform> transforms{};
        Material* tmp; // TODO replace with map of all materials
        
        registry.each([&transforms, &tmp](const transform_3d& t, const render_mesh& m){
            auto transform = Utils::Math::calc_transform_3d(t.position_, t.rotation_, t.scale_);
            auto normal = Utils::Math::calc_normal_matrix(t.rotation_, t.scale_);
            transforms.push_back({transform, normal});
            tmp = m.material_.get();
        });

        int instance = 0;

        tmp->set_uniform_data("objectBuffer", sizeof(transforms[0]) * transforms.size(), transforms.data());
        tmp->set_uniform_data("globalData", sizeof(global_3d_data), &global_3d_data);
        tmp->bind(commandBuffer);

        registry.each([&commandBuffer, &instance](const transform_3d& t, const render_mesh& m){
            m.material_->bind(commandBuffer);
            m.model_->bind(commandBuffer);
            m.model_->draw(commandBuffer, instance);
            instance += 1;
        });
    }

    void Renderer::update_global_ubo(Scene* scene, GlobalData* global_3d_data) {
        auto light_data = scene->get_pointlights();
        uint16_t size = light_data.size();

        global_3d_data->light_index = size;
        for(int i = 0; i < size; ++i) {
            global_3d_data->lightData[i] = {light_data[i]->color, light_data[i]->ambient, light_data[i]->position};
        }
    }
}
