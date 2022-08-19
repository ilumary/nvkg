#pragma once

#include <nvkg/Utils/Math.hpp>
#include <nvkg/Window/Window.hpp>

#include <map>

#define KEY_W GLFW_KEY_W
#define KEY_A GLFW_KEY_A
#define KEY_S GLFW_KEY_S
#define KEY_D GLFW_KEY_D
#define KEY_Q GLFW_KEY_Q
#define KEY_E GLFW_KEY_E
#define KEY_UP GLFW_KEY_UP
#define KEY_DOWN GLFW_KEY_DOWN
#define KEY_RIGHT GLFW_KEY_RIGHT
#define KEY_LEFT GLFW_KEY_LEFT

#define KEY_ESCAPE GLFW_KEY_ESCAPE

class Input {

    public:

        struct MouseCoordinates {
            double x;
            double y;
        };

        static void init_with_window_pointer(nvkg::Window* window);
        static bool key_down(int key);
        static bool key_just_pressed(int key);

        static const MouseCoordinates& get_cursor_pos();
        static MouseCoordinates get_norm_mouse_pos();

    private:
        static nvkg::Window* window;
        static bool detected_motion;
        static MouseCoordinates crnt_mouse_coords;
        static std::map<int, int> keys;

        static void GetCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

};
