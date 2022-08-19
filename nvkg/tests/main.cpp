#define VOLK_IMPLEMENTATION

#include <nvkg/Window/Window.hpp>
#include <nvkg/Renderer/Renderer.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Components/Shape.hpp>
#include <nvkg/Input/Input.hpp>
#include <nvkg/Utils/Math.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Shader/Shader.hpp>
#include <nvkg/Renderer/Lights/PointLight.hpp>
#include <nvkg/Renderer/Scene/Scene.hpp>
#include <nvkg/Renderer/Texture/TextureManager.hpp>

#include <vector>
#include <chrono>
#include <cmath>

static const constexpr int WIDTH = 1280;
static const constexpr int HEIGHT = 720;

void mov_cam_xz(float deltaTime, Components::Shape& viewerObject) {
    static auto oldMousePos = Input::get_cursor_pos();
    auto mousePos = Input::get_cursor_pos();

    glm::vec3 rotate{0};
    float lookSpeed = 4.0f;

    float differenceX = mousePos.x - oldMousePos.x;
    float differenceY = oldMousePos.y - mousePos.y;

    rotate.y += differenceX;
    rotate.x += differenceY;

    if (glm::dot(rotate, rotate) > glm::epsilon<float>()) {
        glm::vec3 newRotation = viewerObject.get_rot() + lookSpeed * glm::normalize(rotate);
        viewerObject.set_rot(Utils::Math::Lerp(viewerObject.get_rot(), newRotation, deltaTime));
    }

    // Limit the pitch values to avoid objects rotating upside-down.
    viewerObject.set_rot_x(glm::clamp(viewerObject.get_rot().x, -1.5f, 1.5f));
    viewerObject.set_rot_y(glm::mod(viewerObject.get_rot().y, glm::two_pi<float>()));

    float yaw = viewerObject.get_rot().y;
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
        glm::vec3 newMove = viewerObject.get_pos() + moveSpeed * glm::normalize(moveDir);
        viewerObject.set_pos(Utils::Math::Lerp(viewerObject.get_pos(), newMove, deltaTime));
    }

    oldMousePos = mousePos;
}

int main() {
    nvkg::Window window("NVKG", WIDTH, HEIGHT);
    window.disable_cursor();
    Input::init_with_window_pointer(&window);

    nvkg::Renderer renderer(window);

    nvkg::TextureManager* tex_mng = new nvkg::TextureManager();
    tex_mng->init(&renderer.get_device());

    nvkg::Camera camera;
    Components::Shape cam_obj;

    nvkg::Scene* scene = new nvkg::Scene("scene1");

    nvkg::ShaderModule *vert_shader = new nvkg::ShaderModule();
    vert_shader->create("simpleShader", "vert");

    nvkg::ShaderModule *frag_shader = new nvkg::ShaderModule();
    frag_shader->create("diffuseFragShader", "frag");

    // Material Declaration
    nvkg::NVKGMaterial diffuse_mat_new(vert_shader, frag_shader); // 3D diffuse test material

    nvkg::SampledTexture* tex = tex_mng->load_2d_img("../assets/textures/tex1.png");
    //diffuse_mat_new.set_texture(tex, "penguin", 2, VK_SHADER_STAGE_FRAGMENT_BIT);

    nvkg::NVKGMaterial::create_materials({&diffuse_mat_new});

    // Generating models from .obj files
    nvkg::Model cubeObjModel("assets/models/cube.obj");
    nvkg::Model vaseObjModel("assets/models/smooth_vase.obj");

    // Set 3D diffuse material
    cubeObjModel.set_material(&diffuse_mat_new);
    vaseObjModel.set_material(&diffuse_mat_new);

    std::vector<Components::Shape> shapes = {
        Components::Shape(&cubeObjModel),
        Components::Shape(&cubeObjModel),
        Components::Shape(&vaseObjModel)
    };

    shapes[0].set_pos({0.f, -.5f, 0.f});
    shapes[0].set_scale({.5f, .5f, .5f});
    shapes[0].set_color({.5f, 0.f, 0.f});

    shapes[1].set_pos({0.f, 0.f, 0.f});
    shapes[1].set_scale({3.f, 3.f, 0.05f});
    shapes[1].set_color({.5f, 0.f, 0.f});
    shapes[1].set_rot_x(1.570796f);

    shapes[2].set_pos({0.f, -1.f, 0.f});
    shapes[2].set_scale({2.f, 2.f, 2.f});
    shapes[2].set_color({.5f, .9f, 0.f});

    cam_obj.set_pos({0.f, -1.f, -2.5f});

    nvkg::PointLightInit light1 = {
        {1.0f, -1.5f, -1.5f}, 
        0.05f, 
        {1.f, 0.f, 0.f, 1.f}, 
        {1.f, 1.f, 1.f, .02f},
    };

    nvkg::PointLightInit light2 = {
        {-1.f, -1.5f, -1.5f}, 
        0.05f, 
        {0.f, 1.f, 0.f, 1.f}, 
        {1.f, 1.f, 1.f, .02f},
    };

    nvkg::PointLightInit light3 = {
        {-0.f, -1.5f, 1.5f}, 
        0.05f, 
        {0.f, 0.f, 1.f, 1.f}, 
        {1.f, 1.f, 1.f, .02f},
    };

    scene->add_shape_3d(&shapes[0], 3);
    scene->add_pointlight(&light1);
    scene->add_pointlight(&light2);
    scene->add_pointlight(&light3);

    auto currentTime = std::chrono::high_resolution_clock::now();

    bool input_enabled = true;

    renderer.set_camera(&camera);

    while(!window.window_should_close()) {
        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        auto alpha = std::clamp<float>(abs(sin(glfwGetTime())), 0.001f, 1.f);

        window.update();

        if (Input::key_just_pressed(KEY_ESCAPE)) {
            input_enabled = !input_enabled;
            window.toggle_cursor(input_enabled);
        }

        float aspect = renderer.get_aspect_ratio();

        camera.set_perspective_proj(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (input_enabled) {
            mov_cam_xz(frameTime, cam_obj);
            camera.set_view_xyz(cam_obj.get_pos(), cam_obj.get_rot());
        }

        if (!renderer.start_frane()) continue;

        scene->draw();
        
        renderer.end_frame();
    }

    renderer.clear_device_queue();

    return 0;
}
