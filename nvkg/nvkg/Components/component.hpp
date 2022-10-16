#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>
#include <map>
#include <string>
#include <functional>

#include <nvkg/Utils/logger.hpp>
#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>

class Component {
    public:

        Component(std::string s) : name(s) {};
        virtual ~Component() {};

        //every BaseType implements basic methods, TODO make scriptable
        [[nodiscard]] virtual bool _on_load() = 0;
        virtual void _on_delete() = 0; 

        //update logic is passed to child object as lambda
        void _on_update(std::function<void()> func) { _on_update_func = func; }
        void _on_fixed_update(std::function<void(float)> func); //TODO physics

        void _update() {
            _on_update_func();
            for(auto& c : components) {
                c.second->_update();
            }
        }

        void _attach_component(Component* c) {
            if(!c->_on_load()) {
                logger::debug() << "Failed to load " << c->name;
                return;
            }
            components[c->name] = c;
        }

        const std::string name;

    private:

        //holds all child objects
        std::map<std::string, Component*> components;

        //normal update method
        std::function<void ()> _on_update_func = [](){};

        //fixed update method for physics
        std::function<void (float)> _fixed_update = [](float){};
};

namespace nvkg {

    class PointLight : public Component {
        public:
            glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 0.2f);
            glm::vec4 ambient = glm::vec4(1.f, 1.f, 1.f, .02f);
            glm::vec3 position = glm::vec3(0.f);
            float radius;

            PointLight(std::string name, glm::vec4 color, glm::vec4 ambient, glm::vec3 position, float radius)
                : Component(name) {
                this->color = color; this->ambient = ambient; this->position = position; this->radius = radius;
            }

            virtual bool _on_load() { return true; }
            virtual void _on_delete() {}
    };

    class UIComponent : public Component {
        public:
            const Mesh::MeshData* mesh;
            Model ui_model;

            glm::vec2 scale;
            glm::vec2 translate;

            //add transform, scale, options to move by [x, y],...
            //create static functions which return mesh of desired object:
            //  UIComponent c ("button", UIComponent::Button(height, width, _on_press))...

            // construct with either Vertex2D or MeshData where size == sizeof(Vertex2D) => should be 28
            UIComponent(std::string name, const Mesh::MeshData* mesh) : Component(name) {
                if(mesh->vertexSize != sizeof(Vertex2D)) {
                    logger::debug(logger::Level::Error) << "UIComponent " << name << " has wrong vertex type and will probably malfunction";
                }
                this->mesh = mesh;
                ui_model.set_mesh(*mesh);

                scale = {1.f, 1.f};
                translate = {0.f, 0.f};
            }
            
            virtual bool _on_load() { return true; }
            virtual void _on_delete() {}
    };

    class SDFText : public Component {
        public:
            
            SDFText(std::string name, std::string text) : Component(name) {

            }

            virtual bool _on_load() { return true; }
            virtual void _on_delete() {}

    };

}

#endif // COMPONENT_HPP
