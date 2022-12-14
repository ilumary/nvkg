#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <string>
#include <functional>

#include <nvkg/Utils/logger.hpp>
#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Components/sdf_text.hpp>

namespace nvkg {
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

    struct point_light {
        glm::vec4 color_;
        alignas(16) glm::vec3 position_;
    };

    struct sdf_text_outline {
        float outline_width_;
        float outline_enabled_;
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
