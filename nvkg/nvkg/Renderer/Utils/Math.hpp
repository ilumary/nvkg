#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace nvkg::Utils {
    class Math {
        public:
        static glm::mat4 calc_transform_3d(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
        static glm::mat2 calc_transform_2d(const glm::vec2& position, const float& rotation, const glm::vec2& scale);
        static glm::mat3 calc_normal_matrix(const glm::vec3& rotation, const glm::vec3& scale);
    };    
}
