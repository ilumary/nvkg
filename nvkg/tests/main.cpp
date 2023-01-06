#define VOLK_IMPLEMENTATION

#include <nvkg/Window/Window.hpp>
#include <nvkg/Renderer/Context.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Input/Input.hpp>
#include <nvkg/Utils/Math.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Shader/Shader.hpp>
#include <nvkg/Renderer/Scene/Scene.hpp>
#include <nvkg/Renderer/Texture/TextureManager.hpp>
#include <nvkg/Components/component.hpp>

#include <vector>
#include <chrono>
#include <cmath>

static const constexpr int WIDTH = 1280;
static const constexpr int HEIGHT = 720;

void mov_cam_xz(float deltaTime, nvkg::transform_3d& cam_transform) {
    static auto oldMousePos = Input::get_cursor_pos();
    auto mousePos = Input::get_cursor_pos();

    glm::vec3 rotate{0};
    float lookSpeed = 4.0f;

    float differenceX = mousePos.x - oldMousePos.x;
    float differenceY = oldMousePos.y - mousePos.y;

    rotate.y += differenceX;
    rotate.x += differenceY;

    if (glm::dot(rotate, rotate) > glm::epsilon<float>()) {
        glm::vec3 newRotation = cam_transform.rotation_ + lookSpeed * glm::normalize(rotate);
        cam_transform.rotation_ = Utils::Math::Lerp(cam_transform.rotation_, newRotation, deltaTime);
    }

    // Limit the pitch values to avoid objects rotating upside-down
    cam_transform.rotation_.x = glm::clamp(cam_transform.rotation_.x, -1.5f, 1.5f);
    cam_transform.rotation_.y = glm::mod(cam_transform.rotation_.y, glm::two_pi<float>());

    float yaw = cam_transform.rotation_.y;
    const glm::vec3 forwardDir{glm::sin(yaw), 0.f, glm::cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (Input::key_down(KEY_W)) moveDir += forwardDir;
    if (Input::key_down(KEY_S)) moveDir -= forwardDir;
    if (Input::key_down(KEY_A)) moveDir -= rightDir;
    if (Input::key_down(KEY_D)) moveDir += rightDir;

    if (Input::key_down(KEY_Q)) moveDir += upDir;
    if (Input::key_down(KEY_E)) moveDir -= upDir;

    float moveSpeed = 2.f;

    if (glm::dot(moveDir, moveDir) > glm::epsilon<float>()) {
        glm::vec3 newMove = cam_transform.position_ + moveSpeed * glm::normalize(moveDir);
        cam_transform.position_ = Utils::Math::Lerp(cam_transform.position_, newMove, deltaTime);
    }

    oldMousePos = mousePos;
}

int main() {
    logger::debug() << alloc_calls_ << ", " << dealloc_calls_ << ", " << used_memory_;
    nvkg::Window window("NVKG", WIDTH, HEIGHT);
    window.disable_cursor();
    Input::init_with_window_pointer(&window); //TODO hide

    nvkg::Context context(window);
    ecs::registry& registry = context.get_registry();

    //TODO need to update camera
    nvkg::Camera camera;
    nvkg::transform_3d cam_transform{};

    //TODO request scene from context
    nvkg::Scene* scene = context.create_scene("scene1");
    scene->set_camera(&camera);

    nvkg::Material diffuse_mat_new({
        .shaders = {"simpleShader.vert", "diffuseFragShader.frag"},
        .textures = {{"texSampler", nvkg::TextureManager::load_2d_img("../assets/textures/tex1.png")}},
    });

    nvkg::Material sdf_mat_new({
        .shaders = {"sdf.vert", "sdf.frag"},
        .textures = {{"samplerColor", nvkg::TextureManager::load_2d_img("../assets/textures/font_sdf_rgba.png")}},
        .pipeline_configurator = [](nvkg::PipelineInit& pipeline) -> void {
            pipeline.rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            pipeline.blend_attachment_state.blendEnable = VK_TRUE;
            pipeline.blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            pipeline.blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            pipeline.blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
            pipeline.blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            pipeline.blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            pipeline.blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
            pipeline.blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            pipeline.depth_stencil_state.depthTestEnable = VK_FALSE;
        },
    });

    auto frame_time = registry.create<nvkg::sdf_text_outline, nvkg::render_mesh>(
        { .55f, false, .75f, {-0.99, -0.99}, {.02f, .04f}, 0.f }, {
        .model_ = nvkg::sdf_text::generate_text("Frame Time: 00000 us"), //TODO maybe add puffer
        .material_ = std::unique_ptr<nvkg::Material>(&sdf_mat_new)
    });

    nvkg::render_mesh& frame_time_render_mesh = registry.get<nvkg::render_mesh>(frame_time);

    // Generating models from .obj files
    nvkg::Model cubeObjModel("assets/models/cube.obj");
    nvkg::Model vaseObjModel("assets/models/smooth_vase.obj");

    //cam_obj.set_pos({0.f, -1.f, -2.5f});
    cam_transform.position_ = {0.f, -1.f, -2.5f};

    auto entity = registry.create<nvkg::transform_3d, nvkg::render_mesh>({
        {0.f, -.5f, 0.f}, {.5f, .5f, .5f}, {0.f, 0.f, 0.f}
    }, {
        .model_ = std::unique_ptr<nvkg::Model>(&cubeObjModel),
        .material_ = std::unique_ptr<nvkg::Material>(&diffuse_mat_new)
    });

    auto entity2 = registry.create<nvkg::transform_3d, nvkg::render_mesh>({
        {0.f, 0.f, 0.f}, {3.f, 3.f, 0.05f}, {1.570796f, 0.f, 0.f}
    }, {
        .model_ = std::unique_ptr<nvkg::Model>(&cubeObjModel),
        .material_ = std::unique_ptr<nvkg::Material>(&diffuse_mat_new)
    });

    auto entity3 = registry.create<nvkg::transform_3d, nvkg::render_mesh>({
        {0.f, -1.f, 0.f}, {2.f, 2.f, 2.f}, {0.f, 0.f, 0.f}
    }, {
        .model_ = std::unique_ptr<nvkg::Model>(&vaseObjModel),
        .material_ = std::unique_ptr<nvkg::Material>(&diffuse_mat_new)
    });

    auto point_light_ent1 = registry.create<nvkg::point_light>({{1.f, 0.f, 0.f, 1.f}, {1.0f, -1.5f, -1.5f}});
    auto point_light_ent2 = registry.create<nvkg::point_light>({{0.f, 1.f, 0.f, 1.f}, {-1.f, -1.5f, -1.5f}});
    auto point_light_ent3 = registry.create<nvkg::point_light>({{0.f, 0.f, 1.f, 1.f}, {-0.f, -1.5f, 1.5f}});

    auto currentTime = std::chrono::high_resolution_clock::now();

    bool input_enabled = true;

    logger::debug() << alloc_calls_ << ", " << dealloc_calls_ << ", " << used_memory_;

    float time_1s = 0.f;

    while(!window.window_should_close()) {
        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        time_1s += frameTime;
        currentTime = newTime;

        if(time_1s > 1) {
            std::stringstream sstm;
            sstm << "Frame Time: " << floorf(frameTime * 100000000) / 100 << " us";

            nvkg::sdf_text::update_model_mesh(sstm.str(), frame_time_render_mesh.model_);

            time_1s -= 1;
        }

        window.update();

        if (Input::key_just_pressed(KEY_ESCAPE)) {
            input_enabled = !input_enabled;
            window.toggle_cursor(input_enabled);
        }

        float aspect = context.get_aspect_ratio();

        camera.set_perspective_proj(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (input_enabled) {
            mov_cam_xz(frameTime, cam_transform);
            camera.set_view_xyz(cam_transform.position_, cam_transform.rotation_);
        }

        context.render();
    }

    context.clear_device_queue();

    return 0;
}
