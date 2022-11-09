#include <nvkg/Renderer/Renderer/Renderer3D/LightRenderer.hpp>

namespace nvkg {
    LightRenderer::LightRenderer() {}
    LightRenderer::~LightRenderer() {}

    void LightRenderer::init(const char* globalDataAttributeName, const uint64_t& globalDataSize) {
        glob_data_id = INTERN_STR(globalDataAttributeName);

        light_material = std::unique_ptr<Material>(new Material({
            .shaders = {{"pointLight", "vert"}, {"pointLight", "frag"}},
        }));

        point_light_vertices.insert(point_light_vertices.end(), {{1.f, 1.f}, {1.f, -1.f}, {-1.f, -1.f}, {-1.f, 1.f}});
        point_light_indices.insert(point_light_indices.end(), {0, 1, 3, 1, 2, 3});

        light_model.set_mesh({ 
            sizeof(glm::vec2),
            point_light_vertices.data(),
            static_cast<uint32_t>(point_light_vertices.size()),
            point_light_indices.data(),
            static_cast<uint32_t>(point_light_indices.size())
        });

        light_model.set_material(light_material.get());
    }

    void LightRenderer::destroy() {
        light_material->destroy_material();
        light_model.DestroyModel();
    }

    void LightRenderer::render(VkCommandBuffer& commandBuffer, const uint64_t& globalDataSize, const void* globalData, std::span<PointLight*> pnt_lgts) {
        if (point_light_vertices.size() == 0) return;

        PointLightPushConstants push{};

        light_material->set_uniform_data(glob_data_id, globalDataSize, globalData);
        light_material->bind(commandBuffer);

        for(auto& light : pnt_lgts) {
            push.position = glm::vec4(light->position, 1.f);
            push.color = light->color;
            push.radius = 0.05f;

            light_material->push_constant(commandBuffer, "push", sizeof(PointLightPushConstants), &push);

            light_model.bind(commandBuffer);
            light_model.draw(commandBuffer, 0);
        }
    }

    void LightRenderer::recreate_materials() {
        //light_material.recreate_pipeline();
        //std::cout << "ups" << std::endl;
    }
}
