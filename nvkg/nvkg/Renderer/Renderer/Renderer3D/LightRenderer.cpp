#include <nvkg/Renderer/Renderer/Renderer3D/LightRenderer.hpp>

namespace nvkg {
    LightRenderer::LightRenderer() {}
    LightRenderer::~LightRenderer() {}

    void LightRenderer::init(const char* globalDataAttributeName, const uint64_t& globalDataSize) {
        glob_data_id = INTERN_STR(globalDataAttributeName);
        //light_data_id = INTERN_STR("lightUBO");

        nvkg::ShaderModule *light_vert_shader = new nvkg::ShaderModule();
        light_vert_shader->create("pointLight", "vert");

        nvkg::ShaderModule *light_frag_shader = new nvkg::ShaderModule();
        light_frag_shader->create("pointLight", "frag");

        light_material_new.set_vert_shader_new(light_vert_shader);
        light_material_new.set_frag_shader_new(light_frag_shader);

        light_material_new.create_material();

        point_light_vertices.insert(point_light_vertices.end(), {{1.f, 1.f}, {1.f, -1.f}, {-1.f, -1.f}, {-1.f, 1.f}});
        point_light_indices.insert(point_light_indices.end(), {0, 1, 3, 1, 2, 3});

        light_model.set_mesh({ 
            sizeof(glm::vec2),
            point_light_vertices.data(),
            static_cast<uint32_t>(point_light_vertices.size()),
            point_light_indices.data(),
            static_cast<uint32_t>(point_light_indices.size())
        });

        light_model.set_material(&light_material_new);
    }

    void LightRenderer::destroy() {
        light_material_new.destroy_material();
        light_model.DestroyModel();
    }

    void LightRenderer::add_point_light(const glm::vec3& position, const float& radius, const glm::vec4& colour, const glm::vec4& ambient) {
        point_lights.push_back({colour, ambient, position, radius});
    }

    void LightRenderer::render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData) {
        if (point_light_vertices.size() == 0) return;

        PointLightPushConstants push{};

        light_material_new.set_uniform_data(glob_data_id, globalDataSize, globalData);
        light_material_new.bind(commandBuffer);

        for(auto& light : point_lights) {
            push.position = glm::vec4(light.position, 1.f);
            push.color = light.color;
            push.radius = 0.05f;

            light_material_new.push_constant(commandBuffer, sizeof(PointLightPushConstants), &push);

            light_model.bind(commandBuffer);
            light_model.draw(commandBuffer, 0);
        }
    }

    void LightRenderer::recreate_materials() {
        //light_material_new.recreate_pipeline();
        std::cout << "ups" << std::endl;
    }
}