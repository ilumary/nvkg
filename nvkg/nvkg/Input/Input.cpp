#include <nvkg/Input/Input.hpp>
#include <iostream>
    
nvkg::Window* Input::window = nullptr;
Input::MouseCoordinates Input::crnt_mouse_coords;
bool Input::detected_motion{false};

std::map<int, int> Input::keys;

void Input::init_with_window_pointer(nvkg::Window* p_window) {
    if(p_window == nullptr) { 
        std::exit(1);
    }
    window = p_window;
    glfwSetCursorPosCallback(window->get_glfw_window(), GetCursorPositionCallback);
}

bool Input::key_down(int key) {
    bool hasKey = (glfwGetKey(window->get_glfw_window(), key) == GLFW_PRESS);

    if (keys.find(key) != keys.end()) {
        keys[key] = hasKey ? 1 : 0;
    } else {
        if (hasKey) keys[key] = 1;
    }
    return hasKey;
}

bool Input::key_just_pressed(int key) {
    bool hasKey = glfwGetKey(window->get_glfw_window(), key) == GLFW_PRESS;

    bool keyEntryExists = keys.find(key) != keys.end();
    if (keyEntryExists && hasKey) {
        if (keys[key] == 0) 
        {
            keys[key] = 1;
            return true;
        }
        else return false;
    } else if (hasKey && !keyEntryExists) {
        keys[key] = 1;
        return true;
    } 
    else if (!hasKey && keyEntryExists) keys[key] = 0;

    return false;
}

const Input::MouseCoordinates& Input::get_cursor_pos() {
    return crnt_mouse_coords;
}

Input::MouseCoordinates Input::get_norm_mouse_pos() {
    return {
        glm::clamp<float>(Utils::Math::Normalise(crnt_mouse_coords.x, 0, window->get_width()), -1.f, 1.f),
        glm::clamp<float>(Utils::Math::Normalise(crnt_mouse_coords.y, 0, window->get_height()), -1.f, 1.f)
    };
}

void Input::GetCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    crnt_mouse_coords.x = static_cast<float>(xpos);
    crnt_mouse_coords.y = static_cast<float>(ypos);
}


        