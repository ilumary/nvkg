#include <nvkg/Renderer/Renderer/Renderer3D/ModelRenderer.hpp>

namespace nvkg {

    ModelRenderer::ModelRenderer() {}
    ModelRenderer::~ModelRenderer() {}

    void ModelRenderer::init(const char* globalDataAttributeName, const uint64_t& globalDataSize) {
        globalDataId = INTERN_STR(globalDataAttributeName);
        transformId = INTERN_STR("objectBuffer");
    }

    void ModelRenderer::destroy() {}

    void ModelRenderer::draw_model(Model* model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) {
        models.push_back(model);

        auto transform = Utils::Math::calc_transform_3d(position, rotation, scale);
        auto normal = Utils::Math::calc_normal_matrix(rotation, scale);
        transforms.push_back({transform, normal});
    }

    void ModelRenderer::render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData) {
        if (models.size() == 0) return;

        for (size_t i = 0; i < models.size(); i++) {
            auto& model = models.at(i);

            if (currentMaterial != model->get_material()) {
                currentMaterial = model->get_material();
                currentMaterial->set_uniform_data(transformId, sizeof(transforms[0]) * transforms.size(), transforms.data());
                currentMaterial->set_uniform_data(globalDataId, globalDataSize, globalData);
                currentMaterial->bind(commandBuffer);
            } 

            if (currentModel != model) {
                currentModel = model;
                currentModel->bind(commandBuffer);
            }

            model->draw(commandBuffer, i);
        }

        currentModel = nullptr;
        currentMaterial = nullptr;
    }

    void ModelRenderer::flush() {
        transforms.clear();
        models.clear();
    }

    void ModelRenderer::recreate_materials() {
        //if (currentMaterial) currentMaterial->recreate_pipeline();
        std::cout << "ups" << std::endl;
    }
}