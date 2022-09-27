#include <nvkg/Components/component.hpp>

void RootComponent::_update() {
    _root_update(this);
    for(auto& c : components) {
        c.second->_update();
    }
}
