#ifndef NVKG_CAMERA_HPP
#define NVKG_CAMERA_HPP

#include <nvkg/Renderer/Core.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace nvkg {

    class Camera {
        public:

        struct GPUCameraData {
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 projView;
        };

        void set_orthographic_projection(float left, float right, float top, float bottom, float near, float far);

        void set_perspective_proj(float fovy, float aspect, float near, float far);

        void set_view_dir(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

        void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

        void set_view_xyz(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& get_proj() { return projection_matrix; }
        const glm::mat4& get_view() { return view_matrix; }
        glm::mat4 get_proj_view() { return projection_matrix * view_matrix; }
        GPUCameraData get_cam_data() { return { projection_matrix, view_matrix, get_proj_view() }; }

        private:
        
        glm::mat4 projection_matrix{1.f};
        glm::mat4 view_matrix{1.f};
    };
}

#endif