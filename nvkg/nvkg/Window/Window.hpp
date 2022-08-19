#pragma once

#include <volk/volk.h>
#include <GLFW/glfw3.h>
#include <cstdint>

namespace nvkg {

    class Window {
    public:

        Window(char const* name, int width, int height) : 
        width(width), height(height), name(name) {
            if (!glfw_init) {
                glfwInit();
                glfw_init = true;    
            }
            
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            window = glfwCreateWindow(width, height, name, nullptr, nullptr);

            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            glfwSetWindowUserPointer(window, this);
            glfwSetWindowSizeCallback(window, resize_callback);
            glfw_window_count++;
        }

        Window() : Window("Window", 800, 600) {}

        ~Window() {
            glfwDestroyWindow(window);
            if (--glfw_window_count <= 0) glfwTerminate();
        }

        const int& get_height() { return height; }
        const int& get_width() { return width; }

        VkExtent2D get_window_extent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

        GLFWwindow* get_glfw_window() { return window; }

        // Public Functions

        void update();

        bool window_should_close();

        bool init_window_surface(VkInstance instance, VkSurfaceKHR* surface);

        bool resized() { return b_resized; }

        void reset_resize() { b_resized = false; }

        void enable_cursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

        void disable_cursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }

        void toggle_cursor(bool state) { state ? disable_cursor() : enable_cursor(); }

        void await_events() { glfwWaitEvents(); }

    private:

        static void resize_callback(GLFWwindow* windowPtr, int width, int height);

        static bool glfw_init;
        static size_t glfw_window_count;

        GLFWwindow* window;

        char const* name;

        int width;
        int height;
        bool b_resized = false;
    };
}
