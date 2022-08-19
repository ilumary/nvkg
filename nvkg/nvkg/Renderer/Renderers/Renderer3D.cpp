#include <nvkg/Renderer/Renderers/Renderer3D.hpp>
#include <iostream>

namespace nvkg {

    // static initialisation
    Utils::StringId Renderer3D::globalDataId;

    ModelRenderer Renderer3D::modelRenderer;
    //DebugRenderer3D Renderer3D::debugRenderer;
    //BillboardRenderer Renderer3D::billboardRenderer;
    LightRenderer Renderer3D::lightRenderer;

    Renderer3D::GlobalData Renderer3D::global3DData;

    void Renderer3D::init() {
        globalDataId = INTERN_STR("globalData");

        global3DData.light_index = 0;

        modelRenderer.Initialise("globalData", sizeof(GlobalData));
        //debugRenderer.Initialise("globalData", sizeof(GlobalData));
        //billboardRenderer.Initialise("globalData", sizeof(GlobalData));
        lightRenderer.init("globalData", sizeof(GlobalData));
    }

    void Renderer3D::draw_billboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& colour) {
        //billboardRenderer.DrawBillboard(position, scale, colour);
    }

    void Renderer3D::draw_model(Model* model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) {
        modelRenderer.DrawModel(model, position, scale, rotation);
    }

    void Renderer3D::add_point_light(const glm::vec3& position, const float& radius, const glm::vec4& colour, const glm::vec4& ambientColor) {   
        global3DData.lightData[global3DData.light_index] = {colour, ambientColor, position};
        global3DData.light_index += 1;
        
        lightRenderer.add_point_light(position, radius, colour, ambientColor);
    }

    void Renderer3D::render(VkCommandBuffer& commandBuffer, const CameraData& cameraData) {
        global3DData.cameraData = cameraData;
        uint64_t globalDataSize = sizeof(global3DData);

        modelRenderer.Render(commandBuffer, globalDataSize, &global3DData);
        lightRenderer.render(commandBuffer, globalDataSize, &global3DData);
        //debugRenderer.Render(commandBuffer, globalDataSize, &global3DData);
        //billboardRenderer.Render(commandBuffer, globalDataSize, &global3DData);
    }

    void Renderer3D::draw_line(const glm::vec3& origin, const glm::vec3& destination, const glm::vec3& colour) {
        //debugRenderer.DrawLine(origin, destination, colour);
    }

    void Renderer3D::recreate_materials() {
        modelRenderer.RecreateMaterials();
        lightRenderer.recreate_materials();
    }

    void Renderer3D::flush() {
        modelRenderer.Flush();
        //lightRenderer.flush();
    }

    void Renderer3D::destroy() {
        modelRenderer.Destroy();
        lightRenderer.destroy();
    }
}
