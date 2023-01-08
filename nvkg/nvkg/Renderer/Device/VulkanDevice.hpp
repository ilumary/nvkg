#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Window/Window.hpp>

#include <nvkg/Renderer/Device/Utils/DebugUtilsMessenger.hpp>
#include <nvkg/Renderer/Device/Utils/Extensions.hpp>
#include <nvkg/Renderer/Device/Utils/QueueFamilyIndices.hpp>
#include <nvkg/Renderer/Device/Utils/PhysicalDevice.hpp>
#include <nvkg/Renderer/Device/Utils/SwapChainSupportDetails.hpp>

#include <array>

#if ENABLE_VALIDATION_LAYERS == 1
	#define VALIDATION_LAYERS_ENABLED true
#else
    #define VALIDATION_LAYERS_ENABLED false
#endif

namespace nvkg {

	class vulkan_device_impl {

		static const bool enable_validation_layers = VALIDATION_LAYERS_ENABLED;

		public:

			vulkan_device_impl(nvkg::Window *window);
			vulkan_device_impl();

			~vulkan_device_impl();

			// Deleting move and copy constructors
			vulkan_device_impl(const vulkan_device_impl &) = delete;
			vulkan_device_impl& operator=(const vulkan_device_impl &) = delete;
			vulkan_device_impl(vulkan_device_impl &&) = delete;
			vulkan_device_impl& operator=(vulkan_device_impl &&) = delete;

			VkCommandPool get_command_pool() { return command_pool_; }
			VkDevice device() { return device_; }
			VkSurfaceKHR surface() { return surface_; }

			VkQueue graphics_queue() { return graphics_queue_; }
			VkQueue present_queue() { return present_queue_; }
			VkQueue compute_queue() { return compute_queue_; }

			size_t get_device_alignment() { return properties.limits.minUniformBufferOffsetAlignment; }
			SwapChainSupportDetails::SwapChainSupportDetails get_swapchain_support() { return SwapChainSupportDetails::QuerySupport(physical_device_, surface_); }
			
			uint32_t find_mem_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

			QueueFamilyIndices::QueueFamilyIndices find_phys_queue_families() { return QueueFamilyIndices::find_queue_families(physical_device_, surface_); }
			VkFormat find_supported_format(const VkFormat* candidates, size_t formatCount, VkImageTiling tiling, VkFormatFeatureFlags features);

			VkCommandBuffer begin_single_time_commands();
			void end_single_time_commands(VkCommandBuffer commandBuffer);

			void cpy_buf(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
			void cpy_buf_to_img(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
			void create_img_with_info(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

			VkPhysicalDeviceProperties properties;

		private:

			void create_instance();
			void setup_debug_messenger();
			void create_surface();
			void pick_phys_device();
			void create_logical_device();
			void create_command_pool();

			VkInstance instance_;
			VkDebugUtilsMessengerEXT debug_messenger_;

			VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;

			Window* window_ {nullptr};
			VkCommandPool command_pool_;

			VkDevice device_;
			VkSurfaceKHR surface_;

			VkQueue graphics_queue_, present_queue_, compute_queue_;

			const std::array<const char*, 1> validation_layers = { "VK_LAYER_KHRONOS_validation" };
			const std::array<const char*, 2> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME };
	};

	vulkan_device_impl& device(Window* window = nullptr);
}
