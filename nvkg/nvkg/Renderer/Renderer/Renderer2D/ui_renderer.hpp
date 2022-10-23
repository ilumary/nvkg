#ifndef UI_RENDERER_HPP
#define UI_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Components/component.hpp>

#include <span>

namespace nvkg {

    class UIRenderer {

        public:

            struct UIPushConstant {
                glm::vec2 scale;
                glm::vec2 translate;
            };

            UIRenderer();
            ~UIRenderer();

            void init();

            void destroy();

            void render(VkCommandBuffer& cmdb, std::span<UIComponent*> uic);

            void recreate_materials();

        private:

            std::unique_ptr<NVKGMaterial> ui_material;
    };

}

#endif // UI_RENDERER_HPP
