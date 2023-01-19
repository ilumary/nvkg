#include <nvkg/Input/Input.hpp>
#include <iostream>
    
nvkg::Window* Input::window = nullptr;
std::pair<double, double> Input::crnt_mouse_coords;

void Input::init_with_window_pointer(nvkg::Window* p_window) {
    if(p_window == nullptr) { 
        std::exit(1);
    }
    window = p_window;
    glfwSetCursorPosCallback(window->get_glfw_window(), GetCursorPositionCallback);
}

bool Input::key_down(int key) {
    return (glfwGetKey(window->get_glfw_window(), key) == GLFW_PRESS);
}

bool Input::mouse_button_down(int button) {
    return (glfwGetMouseButton(window->get_glfw_window(), button) == GLFW_PRESS);
}

const std::pair<double, double>& Input::get_cursor_pos() {
    return crnt_mouse_coords;
}

std::pair<double, double> Input::get_norm_mouse_pos() {
    return std::pair<double, double>(
        glm::clamp<float>(Utils::Math::Normalise(crnt_mouse_coords.first, 0, window->get_width()), -1.f, 1.f),
        glm::clamp<float>(Utils::Math::Normalise(crnt_mouse_coords.second, 0, window->get_height()), -1.f, 1.f)
    );
}

void Input::GetCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    crnt_mouse_coords.first = static_cast<float>(xpos);
    crnt_mouse_coords.second = static_cast<float>(ypos);
}


        