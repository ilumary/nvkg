#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>
#include <map>
#include <string>
#include <functional>

#include <nvkg/Utils/logger.hpp>

class Component {
    public:

        virtual ~Component() {};

        //every BaseType implements basic methods, TODO make scriptable
        [[nodiscard]] virtual bool _on_load() = 0;
        virtual void _on_delete() = 0;

        //update logic is passed to child object as lambda
        void _on_update(std::function<void()> func) { _update = func; }
        void _on_fixed_update(std::function<void(float)> func); //TODO physics

        //the actual functions called to update
        std::function<void ()> _update = [](){};
        std::function<void (float)> _fixed_update = [](float){};

        //void attach_component(Component* type);

    //private:

        std::string name;

        //Component* parent = nullptr;
        //std::vector<Component*> components{};
};

class RootComponent {
    public:

        virtual ~RootComponent() {};

        void _on_update(std::function<void(RootComponent*)> func) { _root_update = func; }
        void _update();

        std::function<void (RootComponent*)> _root_update = [](RootComponent*){};

        std::map<std::string, Component*> components;
};

#endif // COMPONENT_HPP