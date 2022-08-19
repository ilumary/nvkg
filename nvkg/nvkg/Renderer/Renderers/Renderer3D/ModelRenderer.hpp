#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>

namespace nvkg {
    class ModelRenderer {
        public:

        ModelRenderer();
        ~ModelRenderer();

        void Initialise(const char* globalDataAttributeName, const uint64_t& globalDataSize);
        void Destroy();

        void DrawModel(Model* model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation);

        void Render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData);

        void Flush();

        void RecreateMaterials();

        private:

        static constexpr size_t MAX_OBJECT_TRANSFORMS = 1000;

        Utils::StringId globalDataId;
        Utils::StringId transformId;

        std::vector<Model::Transform> transforms{};
        std::vector<Model*> models{};

        NVKGMaterial* currentMaterial {nullptr}; 
        Model* currentModel {nullptr};
    };
}