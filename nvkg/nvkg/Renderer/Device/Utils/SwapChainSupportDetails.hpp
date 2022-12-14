#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <vector>

namespace nvkg::SwapChainSupportDetails {
    /**
     * A struct for storing swapchain support information. 
     * This struct stores surface capabilites, surface image formats, and present modes.    
     **/
    struct SwapChainSupportDetails {
        
        VkSurfaceCapabilitiesKHR capabilities {};

        // An array of available surface formats
        std::vector<VkSurfaceFormatKHR> formats{};

        // A array of present modes
        std::vector<VkPresentModeKHR> presentModes{};
        
        bool hasFormats = false;
        bool hasPresentModes = false;
    };

    /**
     * Queries a physical device for compatibility with a window surface. This should allow
     * us to see whether this device can provide a swapchain that can render images correctly. 
     * 
     * @param device the physical device being evaluated
     * @param surface the window surface that images need to be rendered to.
     **/
    SwapChainSupportDetails QuerySupport(VkPhysicalDevice device, VkSurfaceKHR& surface);
}
