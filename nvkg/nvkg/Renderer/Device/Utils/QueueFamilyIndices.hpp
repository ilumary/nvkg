#pragma once

#include <nvkg/Renderer/Core.hpp>

namespace nvkg::QueueFamilyIndices {
    /**
     * A struct storing the indices of our present and graphics queues.  
     * Vulkan stores the queues in an array, the only way to differentiate 
     * between the queues is through their indices.
     **/
    struct QueueFamilyIndices {
        uint32_t graphics_family_;
        uint32_t present_family_;
        uint32_t compute_family;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
    };

    /**
     * Evaluates if we have both the graphics and present family indices present. 
     * @param indices the QueueFamilyIndices struct to be evaluated
     * @returns a boolean that evaulates to true if both a graphics and present family are present. 
     **/
    bool is_complete(QueueFamilyIndices& indices);

    /**
     * Finds all the available queue indices for the given device. 
     * @param device the physical device required for finding queue indices. 
     * @param surface the window surface to render images to. 
     * @returns a QueueFamilyIndices struct containing the queue indices for graphics and presentation.
     **/
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);
}