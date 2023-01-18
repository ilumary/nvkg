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
        float rotation_;
    };

    struct point_light {
        alignas(16) glm::vec4 color_;
        alignas(16) glm::vec3 position_;
    };

    struct sdf_text_outline {
        alignas(4) glm::float32 outline_width_;
        alignas(4) glm::float32 outline_enabled_;
        alignas(4) glm::float32 text_thickness_;
        alignas(8) glm::vec2 position_;
        alignas(8) glm::vec2 scale_;
        alignas(4) glm::float32 rotation_;
    };

    struct shared_render_mesh {
        std::shared_ptr<Model> model_;
        std::shared_ptr<Material> material_;
    };

    struct instance_data {
        std::vector<transform_3d> instance_data_;
        uint32_t instance_count_;
        staged_buffer instance_data_buffer_{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    };
}

#endif // COMPONENT_HPP
