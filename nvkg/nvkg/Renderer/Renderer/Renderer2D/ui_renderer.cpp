#include <nvkg/Renderer/Renderer/Renderer2D/ui_renderer.hpp>

namespace nvkg {

    UIRenderer::UIRenderer() {}
    UIRenderer::~UIRenderer() {}

    void UIRenderer::init() {
        nvkg::ShaderModule *ui_vert_shader = new nvkg::ShaderModule();
        ui_vert_shader->create("ui", "vert");

        nvkg::ShaderModule *ui_frag_shader = new nvkg::ShaderModule();
        ui_frag_shader->create("ui", "frag");

        ui_material.set_vert_shader_new(ui_vert_shader);
        ui_material.set_frag_shader_new(ui_frag_shader);

        ui_material.create_material();
    }

    void UIRenderer::destroy() {
        ui_material.destroy_material();
    }

    void UIRenderer::render(VkCommandBuffer& cmdb, std::span<UIComponent*> uic) {
        if(uic.empty()) return;

        ui_material.bind(cmdb);

        //TODO merge meshes because its all the same pipeline and we can save some draw calls
        for(auto& u : uic) {
            u->ui_model.bind(cmdb);
            u->ui_model.draw(cmdb, 0); 
        }
    }

    void UIRenderer::recreate_materials() {
        //ui_material.recreate_pipeline();
    }

}
