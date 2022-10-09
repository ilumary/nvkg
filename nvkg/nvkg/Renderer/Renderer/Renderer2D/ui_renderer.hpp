#ifndef UI_RENDERER_HPP
#define UI_RENDERER_HPP

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Components/component.hpp>

#include <span>

namespace nvkg {

    class UIRenderer {

        public:

            struct UIComponent : public Component {
                const Mesh::MeshData* mesh;
                Model ui_model;

                // construct with either Vertex2D or MeshData where size == sizeof(Vertex2D) => should be 28
                UIComponent(std::string name, const Mesh::MeshData* mesh) : Component(name) {
                    if(mesh->vertexSize != sizeof(Vertex2D)) {
                        logger::debug(logger::Level::Error) << "UIComponent " << name << " has wrong vertex type and will probably malfunction";
                    }
                    this->mesh = mesh;
                    ui_model.set_mesh(*mesh);
                }
                
                virtual bool _on_load() { return true; }

                virtual void _on_delete() {}
            };

        
            UIRenderer();
            ~UIRenderer();

            void init();

            void destroy();

            void render(VkCommandBuffer& cmdb, std::span<UIComponent*> uic);

            void recreate_materials();

        private:

            NVKGMaterial ui_material;

    };

}

#endif // UI_RENDERER_HPP
