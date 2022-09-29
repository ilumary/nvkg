#include <nvkg/Renderer/Renderer/Renderer3D/ModelRenderer.hpp>

namespace nvkg {

    ModelRenderer::ModelRenderer() {}
    ModelRenderer::~ModelRenderer() {}

    void ModelRenderer::init(const char* globalDataAttributeName, const uint64_t& globalDataSize) {
        globalDataId = INTERN_STR(globalDataAttributeName);
        transformId = INTERN_STR("objectBuffer");
    }

    void ModelRenderer::destroy() {}

    void ModelRenderer::update_models(Components::Shape** shapes, uint16_t count) {
        flush();

        for(uint16_t i = 0; i < count; ++i) {
            models.push_back(shapes[i]->get_model());

            auto transform = Utils::Math::calc_transform_3d(shapes[i]->get_pos(), shapes[i]->get_rot(), shapes[i]->get_scale());
            auto normal = Utils::Math::calc_normal_matrix(shapes[i]->get_rot(), shapes[i]->get_scale());
            transforms.push_back({transform, normal});
        }
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
        //std::cout << "ups" << std::endl;
    }
}
