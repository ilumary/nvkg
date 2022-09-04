#pragma once

#include <nvkg/Renderer/Model/Model.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Components {
    struct Transform {
        glm::vec3 position {0.0f};
        glm::vec3 scale {1.f, 1.f, 1.f};
        glm::vec3 rotation {};
    };

    class Shape {
        public: 

            Shape();
            Shape(nvkg::Model* model);
            ~Shape();

            nvkg::Model::Transform get_transform() { return { calc_transform(transform), calc_normal_matrix(transform) }; };
            glm::vec3& get_color() { return fillColor; }
            nvkg::Model* get_model() { return model; }

            glm::vec3& get_rot() { return transform.rotation; }
            glm::vec3& get_pos() { return transform.position; }
            glm::vec3& get_scale() { return transform.scale; }

            const glm::vec3& get_rot() const { return transform.rotation; }
            const glm::vec3& get_pos() const { return transform.position; }
            const glm::vec3& get_scale() const { return transform.scale; } 
            const float GetZIndex() const { return transform.position.z; }

            void set_color(glm::vec3 newColor);
            void set_scale(glm::vec3 newScale);
            void set_pos(glm::vec3 newPos);
            void set_rot(glm::vec3 rotation);
            void set_rot_x(float rotation);
            void set_rot_y(float rotation);
            void set_rot_z(float rotation);

        private: 

            // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
            // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
            // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
            glm::mat4 calc_transform(Transform& transform);

            glm::mat3 calc_normal_matrix(Transform& transform);
            
            nvkg::Model* model;
            Transform transform{};
            glm::vec3 fillColor{0.0f};
    };
}