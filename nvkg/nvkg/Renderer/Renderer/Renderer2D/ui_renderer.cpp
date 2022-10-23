#include <nvkg/Renderer/Renderer/Renderer2D/ui_renderer.hpp>

namespace nvkg {

    UIRenderer::UIRenderer() {}
    UIRenderer::~UIRenderer() {}

    void UIRenderer::init() {
        ui_material = std::unique_ptr<NVKGMaterial>(new NVKGMaterial({
            .shaders = {{"ui", "vert"}, {"ui", "frag"}},
        }));
    }

    void UIRenderer::destroy() {
        ui_material->destroy_material();
    }

    void UIRenderer::render(VkCommandBuffer& cmdb, std::span<UIComponent*> uic) {
        if(uic.empty()) return;

        UIPushConstant push{};

        ui_material->bind(cmdb);

        for(auto& u : uic) {
            push.scale = u->scale;
            push.translate = u->translate;

            ui_material->push_constant(cmdb, "st", sizeof(UIPushConstant), &push);

            u->ui_model.bind(cmdb);
            u->ui_model.draw(cmdb, 0);
        }
    }

    void UIRenderer::recreate_materials() {
        //ui_material.recreate_pipeline();
    }

}
