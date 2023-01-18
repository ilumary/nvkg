#define VOLK_IMPLEMENTATION

#include <nvkg/Window/Window.hpp>
#include <nvkg/Renderer/Context.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Input/Input.hpp>
#include <nvkg/Utils/Math.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Shader/Shader.hpp>
#include <nvkg/Renderer/Texture/TextureManager.hpp>
#include <nvkg/Components/component.hpp>

#include <vector>
#include <chrono>
#include <cmath>

static const constexpr int WIDTH = 1280;
static const constexpr int HEIGHT = 720;

int main() {
    logger::debug() << alloc_calls_ << ", " << dealloc_calls_ << ", " << used_memory_;
    nvkg::Window window("NVKG", WIDTH, HEIGHT);

    nvkg::Context context(window);
    ecs::registry& registry = context.get_registry();

    std::shared_ptr<nvkg::CameraNew> camera = std::make_shared<nvkg::CameraNew>();
    camera->type = nvkg::CameraNew::CameraType::firstperson;
    camera->setPosition(glm::vec3(0.f, 0.f, -20.f));
    camera->setRotation(glm::vec3(0.f, 0.f, 0.f));
    camera->setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 1.0f, 256.0f);
    camera->setMovementSpeed(5.0f);
    context.set_camera(camera);

    auto frame_time = registry.create<nvkg::sdf_text_outline, nvkg::render_mesh>(
        { .55f, false, .75f, {-0.99, -0.99}, {.02f, .04f}, 0.f }, {
        .model_ = nvkg::sdf_text::generate_text("Frame Time: 00000 us"),
    });

    nvkg::render_mesh& frame_time_render_mesh = registry.get<nvkg::render_mesh>(frame_time);

    auto mem_usage = registry.create<nvkg::sdf_text_outline, nvkg::render_mesh>(
        { .55f, false, .75f, {-0.99, -0.95}, {.02f, .04f}, 0.f }, {
        .model_ = nvkg::sdf_text::generate_text("Memory Usage: 00000 MB"),
    });

    nvkg::render_mesh& mem_usage_render_mesh = registry.get<nvkg::render_mesh>(mem_usage);

    // Generating models from .obj files
    //nvkg::Model cubeObjModel("assets/models/cube.obj");

    /*auto point_light_ent1 = registry.create<nvkg::point_light>({{1.f, 0.f, 0.f, 1.f}, {1.0f, -1.5f, -1.5f}});
    auto point_light_ent2 = registry.create<nvkg::point_light>({{0.f, 1.f, 0.f, 1.f}, {-1.f, -1.5f, -1.5f}});
    auto point_light_ent3 = registry.create<nvkg::point_light>({{0.f, 0.f, 1.f, 1.f}, {-0.f, -1.5f, 1.5f}});*/
    
    /////Instancing

    nvkg::Material::MaterialConfig config_test({
        .shaders = {"instancing.vert", "instancing.frag"},
        .instance_data = { true, sizeof(nvkg::Vertex), sizeof(nvkg::transform_3d) },
    });

    auto instanced_entity = registry.create<nvkg::shared_render_mesh, nvkg::instance_data>({
        .model_ = std::make_shared<nvkg::Model>("assets/models/cube.obj"),
        .material_ = std::make_shared<nvkg::Material>(config_test)
    }, {});

    nvkg::instance_data& instance_data = registry.get<nvkg::instance_data>(instanced_entity);
    instance_data.instance_data_ = {{{0.f, 0.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{2.f, 0.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{-2.f, 0.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{-2.f, -2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{2.f, -2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{0.f, -2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{-2.f, 2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{2.f, 2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}},
                                    {{0.f, 2.f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}}};
    instance_data.instance_count_ = instance_data.instance_data_.size();

    instance_data.instance_data_buffer_.init_staging_buffer(sizeof(nvkg::transform_3d) * instance_data.instance_count_);
    instance_data.instance_data_buffer_.create_buffer(instance_data.instance_data_.data(), sizeof(nvkg::transform_3d) * instance_data.instance_count_);

    /////

    logger::debug() << alloc_calls_ << ", " << dealloc_calls_ << ", " << used_memory_;

    float time_1s = 0.f;

    while(!window.window_should_close()) {
        
        float frameTime = context.get_frame_time();
        time_1s += frameTime;

        if(time_1s > 1) {
            std::stringstream sstm, sstm_m;
            sstm << "Frame Time: " << floorf(frameTime * 100000000) / 100 << " us";

            sstm_m << "Memory Usage: " << used_memory_ / 1000000 << " MB";

            nvkg::sdf_text::update_model_mesh(sstm.str(), frame_time_render_mesh.model_);
            nvkg::sdf_text::update_model_mesh(sstm_m.str(), mem_usage_render_mesh.model_);

            time_1s -= 1;
        }

        window.update();

        context.render();
    }

    context.clear_device_queue();

    return 0;
}
