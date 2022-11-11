#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <span>
#include <variant>

#include <nvkg/Utils/logger.hpp>
#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>

template <typename ... Ts>
using poly_T = std::variant<Ts...>;

template <typename derived_t>
struct component {

    public:

        component(std::string s) : name(s) {};
        ~component() {};

        auto const& derived() const { return static_cast<derived_t const&>(*this); }

        //TODO make scriptable
        bool _on_load() const { return derived()._on_load(); }
        bool _on_delete() const { return derived()._on_delete(); }

        //update logic is passed to child object as lambda
        void _on_update(std::function<void()> func) { _on_update_func = func; }
        void _on_fixed_update(std::function<void(float)> func); //TODO physics

        void _update() {
            _on_update_func();
            for(auto& c : components) {
                c.second->_update();
            }
        }

        void _attach_component(derived_t* c) { //TODO check if name already exists
            if(!c->_on_load()) {
                logger::debug() << "Failed to load " << c->name;
                return;
            }
            components[c->name] = c;
        }

        const std::string name;

    protected:

        //holds all child objects
        std::map<std::string, derived_t*> components;

        //normal update method
        std::function<void ()> _on_update_func = [](){};

        //fixed update method for physics
        std::function<void (float)> _fixed_update = [](float){};
};

namespace nvkg {

    /**
     * Base class for components in 2d space.
     *
     * Every canvas has a position and size. Child objects are positioned relative to the
     * parent object. If a canvas object is updated, every child object is updated by
     * the same amount.
     */
    template<typename derived_t>
    struct canvas_impl : public component<derived_t> {

        canvas_impl(std::string name, glm::vec2 pos, glm::vec2 dimensions) 
            : component<derived_t>(name) {
            //static cast float to int, if == 0 ok, if 1 either 1.0 or >1.0
            this->pos = pos; this->dimensions = dimensions;
        };
        ~canvas_impl() {};

        void update_pos(glm::vec2 pos) {
            this->pos = pos;
            for(auto& [k, v] : component<derived_t>::components) {
                v->update_pos(pos); 
            }
        }

        bool _on_load() const { return true; } //TODO check for parent and position relative to parent
        bool _on_delete() const { return true; }

        protected:
            glm::vec2 pos, dimensions;
            float height, width;
    };

    struct canvas : public canvas_impl<canvas> {
        using canvas_impl<canvas>::canvas_impl;
    };

    /**
     * Base class for components in 3d space.
     *
     * Every spatial node has a position. Child objects are positioned relative to the
     * parent object. If a spatial object is updated, every child object is updated by
     * the same amount.
     */
    template<typename derived_t>
    struct spatial_impl : public component<derived_t> {
        
        spatial_impl(std::string name, glm::vec3 position) : component<derived_t>(name) {
            this->pos = position;  
        };

        protected:
            glm::vec3 pos;
    };

    struct spatial : public spatial_impl<spatial> {
        using spatial_impl<spatial>::spatial_impl;
    };

    class PointLight : public component<PointLight> {
        public:
            glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 0.2f);
            glm::vec4 ambient = glm::vec4(1.f, 1.f, 1.f, .02f);
            glm::vec3 position = glm::vec3(0.f);
            float radius;

            PointLight(std::string name, glm::vec4 color, glm::vec4 ambient, glm::vec3 position, float radius)
                : component(name) {
                this->color = color; this->ambient = ambient; this->position = position; this->radius = radius;
            }

            bool _on_load() { return true; }
            bool _on_delete() { return true; }
    };

    class UIComponent : public component<UIComponent> {
        public:
            const Mesh::MeshData* mesh;
            Model ui_model;

            glm::vec2 scale;
            glm::vec2 translate;

            float outline_width = .1f;

            //add transform, scale, options to move by [x, y],...
            //create static functions which return mesh of desired object:
            //  UIComponent c ("button", UIComponent::Button(height, width, _on_press))...
            //  Make UiComponents in external header, bounding box without renderable content

            // construct with either Vertex2D or MeshData where size == sizeof(Vertex2D) => should be 28
            UIComponent(std::string name, const Mesh::MeshData* mesh) : component(name) {
                if(mesh->vertexSize != sizeof(Vertex2D)) {
                    logger::debug(logger::Level::Error) << "UIComponent " << name << " has wrong vertex type and will probably malfunction";
                }
                this->mesh = mesh;
                ui_model.set_mesh(*mesh);

                scale = {1.f, 1.f};
                translate = {0.f, 0.f};
            }
            
            bool _on_load() { return true; }
            bool _on_delete() { return true; }
    };*/

    /*
    * New Components desinged around Entity Component System
    */

    struct render_mesh { //normal rendering
        std::unique_ptr<Model> model_;
        std::unique_ptr<Material> material_;
    };

    struct transform_3d {
        glm::vec3 position_;
        glm::vec3 scale_;
        glm::vec3 rotation_;
    };

    struct transform_2d {
        glm::vec2 position_;
        glm::vec2 scale_;
        glm::vec2 rotation_;
    };

    struct shared_render_mesh { // instanced rendering, TODO
        const std::shared_ptr<Mesh::MeshData> mesh_;
        const std::shared_ptr<Material> material_;
    };

    struct instance_data {
        std::shared_ptr<transform_3d> instance_data_;
    };
}

#endif // COMPONENT_HPP
