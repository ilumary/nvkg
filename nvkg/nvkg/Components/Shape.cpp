#include <nvkg/Components/Shape.hpp>

namespace Components {
    glm::mat4 Shape::calc_transform(Transform& transform) {
        const float c3 = glm::cos(transform.rotation.z);
        const float s3 = glm::sin(transform.rotation.z);
        const float c2 = glm::cos(transform.rotation.x);
        const float s2 = glm::sin(transform.rotation.x);
        const float c1 = glm::cos(transform.rotation.y);
        const float s1 = glm::sin(transform.rotation.y);

        return glm::mat4{
        {
            transform.scale.x * (c1 * c3 + s1 * s2 * s3),
            transform.scale.x * (c2 * s3),
            transform.scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            transform.scale.y * (c3 * s1 * s2 - c1 * s3),
            transform.scale.y * (c2 * c3),
            transform.scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            transform.scale.z * (c2 * s1),
            transform.scale.z * (-s2),
            transform.scale.z * (c1 * c2),
            0.0f,
        },
        {transform.position.x, transform.position.y, transform.position.z, 1.0f}};
    }

    glm::mat3 Shape::calc_normal_matrix(Transform& transform) {
        const float c3 = glm::cos(transform.rotation.z);
        const float s3 = glm::sin(transform.rotation.z);
        const float c2 = glm::cos(transform.rotation.x);
        const float s2 = glm::sin(transform.rotation.x);
        const float c1 = glm::cos(transform.rotation.y);
        const float s1 = glm::sin(transform.rotation.y);

        glm::vec3 inverseScale = 1.0f / transform.scale;

        return glm::mat3{
            {
                inverseScale.x * (c1 * c3 + s1 * s2 * s3),
                inverseScale.x * (c2 * s3),
                inverseScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                inverseScale.y * (c2 * c3),
                inverseScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                inverseScale.z * (c2 * s1),
                inverseScale.z * (-s2),
                inverseScale.z * (c1 * c2),
            }
        };
    }

    Shape::Shape() {}

    Shape::Shape(nvkg::Model* model) : model{model} {}

    Shape::~Shape() {}

    void Shape::set_color(glm::vec3 newColor) { fillColor = newColor; }
    void Shape::set_pos(glm::vec3 newPos) { transform.position = newPos; }
    void Shape::set_scale(glm::vec3 newScale) { transform.scale = newScale; }
    void Shape::set_rot(glm::vec3 rotation) { transform.rotation = rotation; }
    void Shape::set_rot_x(float rotation) { transform.rotation.x = rotation; }
    void Shape::set_rot_y(float rotation) { transform.rotation.y = rotation; }
    void Shape::set_rot_z(float rotation) { transform.rotation.z = rotation; }
}