#include <nvkg/Window/Window.hpp>

namespace nvkg {
    bool Window::glfw_init = false;
    size_t Window::glfw_window_count = 0;

    void Window::update() {
        glfwPollEvents();
    }

    bool Window::window_should_close() {
        return glfwWindowShouldClose(window);
    }

    bool Window::init_window_surface(VkInstance instance, VkSurfaceKHR* surface) {
        return glfwCreateWindowSurface(instance, window, nullptr, surface) == VK_SUCCESS;
    }

    void Window::resize_callback(GLFWwindow* windowPtr, int width, int height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr));

        window->b_resized = true;
        
        window->width = width;
        window->height = height;
    }
}