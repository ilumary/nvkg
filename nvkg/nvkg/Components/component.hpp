#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>
#include <map>
#include <string>
#include <functional>

#include <nvkg/Utils/logger.hpp>

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

#endif // COMPONENT_HPP
