#include <nvkg/Renderer/Renderer/Renderer.hpp>
#include <iostream>

namespace nvkg {

    // static initialisation
    Utils::StringId Renderer::globalDataId;

    ModelRenderer Renderer::model_renderer;
    LightRenderer Renderer::light_renderer;

    Renderer::GlobalData Renderer::global3DData;

    void Renderer::init() {
        globalDataId = INTERN_STR("globalData");

        global3DData.light_index = 0;

        model_renderer.init("globalData", sizeof(GlobalData));
        light_renderer.init("globalData", sizeof(GlobalData));
    }

    void Renderer::draw_billboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& colour) {
        //billboardRenderer.DrawBillboard(position, scale, colour);
    }

    void Renderer::draw_model(Model* model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) {
        model_renderer.draw_model(model, position, scale, rotation);
    }

    void Renderer::add_point_light(const glm::vec3& position, const float& radius, const glm::vec4& colour, const glm::vec4& ambientColor) {   
        global3DData.lightData[global3DData.light_index] = {colour, ambientColor, position};
        global3DData.light_index += 1;
        
        light_renderer.add_point_light(position, radius, colour, ambientColor);
    }

    void Renderer::render(VkCommandBuffer& commandBuffer, const CameraData& cameraData) {
        global3DData.cameraData = cameraData;
        uint64_t globalDataSize = sizeof(global3DData);

        model_renderer.render(commandBuffer, globalDataSize, &global3DData);
        light_renderer.render(commandBuffer, globalDataSize, &global3DData);
    }

    void Renderer::draw_line(const glm::vec3& origin, const glm::vec3& destination, const glm::vec3& colour) {
        //debugRenderer.DrawLine(origin, destination, colour);
    }

    void Renderer::recreate_materials() {
        model_renderer.recreate_materials();
        light_renderer.recreate_materials();
    }

    void Renderer::flush() {
        model_renderer.flush();
    }

    void Renderer::destroy() {
        model_renderer.destroy();
        light_renderer.destroy();
    }
}
