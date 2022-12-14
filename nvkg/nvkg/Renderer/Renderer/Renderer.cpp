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
        });
    }

    void Renderer::recreate_materials() {

    }

    void Renderer::destroy() {

    }

    void Renderer::render(VkCommandBuffer& commandBuffer, Scene* scene, const ecs::registry& registry) {
        GlobalData global_3d_data{};
        global_3d_data.light_index = 0;
        global_3d_data.cameraData = scene->get_camera_data();

        registry.each([&global_3d_data](const point_light& p){
            global_3d_data.light_data[global_3d_data.light_index] = p;
            global_3d_data.light_index += 1;
        });

        uint64_t globalDataSize = sizeof(global_3d_data);

        std::vector<Model::Transform> transforms{};
        Material* tmp; // TODO replace with map of all materials
        
        registry.each([&transforms, &tmp](const transform_3d& t, const render_mesh& m){
            auto transform = Utils::Math::calc_transform_3d(t.position_, t.rotation_, t.scale_);
            auto normal = Utils::Math::calc_normal_matrix(t.rotation_, t.scale_);
            transforms.push_back({transform, normal});
            tmp = m.material_.get();
        });

        int instance = 0;

        tmp->set_uniform_data("objectBuffer", sizeof(transforms[0]) * transforms.size(), transforms.data());
        tmp->set_uniform_data("globalData", sizeof(global_3d_data), &global_3d_data);
        tmp->bind(commandBuffer);

        registry.each([&commandBuffer, &instance](const transform_3d& t, const render_mesh& m){
            m.material_->bind(commandBuffer);
            m.model_->bind(commandBuffer);
            m.model_->draw(commandBuffer, instance);
            instance += 1;
        });

        tmp = light_material.get();
        Model* m = &light_model;

        tmp->set_uniform_data("globalData", sizeof(global_3d_data), &global_3d_data);
        tmp->bind(commandBuffer);

        registry.each([&commandBuffer, &tmp, &m](const point_light& p){
            tmp->push_constant(commandBuffer, "push", sizeof(point_light), &p);
            m->bind(commandBuffer);
            m->draw(commandBuffer, 0);
        });
        
        registry.each([&commandBuffer](const sdf_text_outline& s, const render_mesh& r) {
            r.material_->bind(commandBuffer);
            r.material_->push_constant(commandBuffer, "push", sizeof(sdf_text_outline), &s);
            r.model_->bind(commandBuffer);
            r.model_->draw(commandBuffer, 0);
        });
    }
}
