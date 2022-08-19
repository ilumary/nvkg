#include <nvkg/Renderer/Device/Utils/SwapChainSupportDetails.hpp>

namespace nvkg::SwapChainSupportDetails {
    SwapChainSupportDetails QuerySupport(VkPhysicalDevice device, VkSurfaceKHR& surface) {
        SwapChainSupportDetails details;
        // Populate the surface capabilities value
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, OUT &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, OUT &formatCount, nullptr);

        if (formatCount != 0) {
            // Populate our format array
            details.formats = std::vector<VkSurfaceFormatKHR>(static_cast<size_t>(formatCount));
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            
            details.hasFormats = true;
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, OUT &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            // populate our presentModes array
            details.presentModes = std::vector<VkPresentModeKHR>(static_cast<uint32_t>(presentModeCount));
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface,
                &presentModeCount,
                details.presentModes.data());

            details.hasPresentModes = true;
        }
        return details;
    }
}
