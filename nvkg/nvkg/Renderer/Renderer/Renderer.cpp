#include <nvkg/Renderer/Renderer/Renderer.hpp>

namespace nvkg {

    Renderer::Renderer() {
        global_data_id = INTERN_STR("globalData");

        global_3d_data.light_index = 0;

        model_renderer = new ModelRenderer();
        model_renderer->init("globalData", sizeof(GlobalData));

        light_renderer = new LightRenderer();
        light_renderer->init("globalData", sizeof(GlobalData));

        ui_renderer = new UIRenderer();
        ui_renderer->init();
    }

    void Renderer::recreate_materials() {
        model_renderer->recreate_materials();
        light_renderer->recreate_materials();
    }

    void Renderer::destroy() {
        model_renderer->destroy();
        light_renderer->destroy();
    }

    void Renderer::render(VkCommandBuffer& commandBuffer, Scene* scene) {
        if(scene->updated) {
            update_global_ubo(scene);
            scene->updated = false;
        }

        global_3d_data.cameraData = scene->get_camera_data();
        uint64_t globalDataSize = sizeof(global_3d_data);

        model_renderer->render(commandBuffer, globalDataSize, &global_3d_data, scene->get_3d_shapes());
        light_renderer->render(commandBuffer, globalDataSize, &global_3d_data, scene->get_pointlights());
        ui_renderer->render(commandBuffer, scene->get_ui_components());
    }

    void Renderer::update_global_ubo(Scene* scene) {
        auto light_data = scene->get_pointlights();
        uint16_t size = light_data.size();

        global_3d_data.light_index = size;
        for(int i = 0; i < size; ++i) {
            global_3d_data.lightData[i] = {light_data[i]->color, light_data[i]->ambient, light_data[i]->position};
        }
    }
}
