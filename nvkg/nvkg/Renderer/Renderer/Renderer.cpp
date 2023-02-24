#include <nvkg/Renderer/Renderer/Renderer.hpp>

namespace nvkg {

    Renderer::Renderer() {
        global_data_id = INTERN_STR("globalData");
        transform_id = INTERN_STR("objectBuffer");

        light_material = std::unique_ptr<Material>(new Material({
            .shaders = {"pointLight.vert", "pointLight.frag"},
        }));

        point_light_vertices.insert(point_light_vertices.end(), {{1.f, 1.f}, {1.f, -1.f}, {-1.f, -1.f}, {-1.f, 1.f}});
        point_light_indices.insert(point_light_indices.end(), {0, 1, 3, 1, 2, 3});

        light_model.set_mesh({ 
            sizeof(glm::vec2),
            point_light_vertices.data(),
            static_cast<uint32_t>(point_light_vertices.size()),
            point_light_indices.data(),
            static_cast<uint32_t>(point_light_indices.size())
        });*/
    }

    Renderer::~Renderer() {}

    void Renderer::recreate_materials() {}

    void Renderer::render(VkCommandBuffer& commandBuffer, std::shared_ptr<CameraNew> camera, ecs::registry& registry) {
        const auto render_sys = [&](const shared_render_mesh& srm, const instance_data& id){
            struct ubo {
                glm::mat4 projection;
                glm::mat4 modelview;
                glm::vec4 light_pos = {0.0f, -5.0f, 0.0f, 1.0f};
            } ubo;

            ubo.projection = camera->matrices.perspective;
            ubo.modelview = camera->matrices.view;

            MaterialManager::get(srm.material_)->set_uniform_data("globalData", sizeof(ubo), &ubo);
            MaterialManager::get(srm.material_)->bind(commandBuffer);
            srm.model_->bind(commandBuffer, VERTEX_BUFFER_BIND_ID);
            
            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, INSTANCE_BUFFER_BIND_ID, 1, &id.instance_data_buffer_.buffer_.buffer, offsets);
            
            vkCmdDrawIndexed(commandBuffer, srm.model_->get_index_count(), id.instance_count_, 0, 0, 0);
        };

        registry.each(render_sys);

        /*tmp = light_material.get();
        Model* m = &light_model;

        tmp->set_uniform_data("globalData", sizeof(global_3d_data), &global_3d_data);
        tmp->bind(commandBuffer);

        registry.each([&commandBuffer, &tmp, &m](const point_light& p){
            tmp->push_constant(commandBuffer, "push", sizeof(point_light), &p);
            m->bind(commandBuffer);
            m->draw(commandBuffer, 0);
        });*/

        material_handle tmp = sdf_text::sdf_material();
        MaterialManager::get(tmp)->bind(commandBuffer);

        const auto sdf_sys = [&commandBuffer, &tmp](const sdf_text_outline& s, const render_mesh& r) {
            MaterialManager::get(tmp)->push_constant(commandBuffer, "push", sizeof(sdf_text_outline), &s);
            r.model_->bind(commandBuffer);
            r.model_->draw(commandBuffer, 0);
        };

        registry.each(sdf_sys);
    }
}
