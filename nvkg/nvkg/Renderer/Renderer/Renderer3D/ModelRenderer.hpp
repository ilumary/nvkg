#ifndef NVKG_MODEL_RENDERER_HPP
#define NVKG_MODEL_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Utils/Math.hpp>
#include <nvkg/Components/Shape.hpp>

#include <span>

namespace nvkg {
    class ModelRenderer {
        public:

            ModelRenderer();
            ~ModelRenderer();

            void init(const char* globalDataAttributeName, const uint64_t& globalDataSize);
            void destroy();

            void render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData, std::span<Components::Shape*> shapes);

            void flush();

            void recreate_materials();

        private:

            Utils::StringId globalDataId;
            Utils::StringId transformId;

            std::vector<Model::Transform> transforms{};
            std::vector<Model*> models{};

            Material* currentMaterial {nullptr}; 
            Model* currentModel {nullptr};
    };
}

#endif
