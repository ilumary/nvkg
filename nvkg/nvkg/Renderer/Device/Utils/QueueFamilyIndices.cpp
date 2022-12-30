#include <nvkg/Renderer/Device/Utils/QueueFamilyIndices.hpp>

namespace nvkg::QueueFamilyIndices {
    bool is_complete(QueueFamilyIndices& indices) { return indices.graphicsFamilyHasValue && indices.presentFamilyHasValue; }

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, OUT &queueFamilyCount, nullptr);

        VkQueueFamilyProperties queueFamilies[queueFamilyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, OUT queueFamilies);

        for (size_t i = 0; i < queueFamilyCount; i++) {
            VkQueueFamilyProperties queueFamily = queueFamilies[i];
            
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family_ = i;
                indices.graphicsFamilyHasValue = true;
            }

            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                indices.compute_family = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, OUT &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.present_family_ = i;
                indices.presentFamilyHasValue = true;
            }
            if (is_complete(indices)) {
                break;
            }
        }
        return indices;
    }
}