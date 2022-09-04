#ifndef NVKG_MODEL_RENDERER_HPP
#define NVKG_MODEL_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>
#include <nvkg/Components/Shape.hpp>

namespace nvkg {
    class ModelRenderer {
        public:

            ModelRenderer();
            ~ModelRenderer();

            void init(const char* globalDataAttributeName, const uint64_t& globalDataSize);
            void destroy();

            void draw_model(Model* model, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation);

            void render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData);

            void update_models(Components::Shape* shapes, uint16_t count);

            void flush();

            void recreate_materials();

        private:

            Utils::StringId globalDataId;
            Utils::StringId transformId;

            std::vector<Model::Transform> transforms{};
            std::vector<Model*> models{};

            NVKGMaterial* currentMaterial {nullptr}; 
            Model* currentModel {nullptr};
    };
}

#endif