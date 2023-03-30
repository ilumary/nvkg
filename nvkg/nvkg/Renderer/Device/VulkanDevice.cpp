#include <nvkg/Renderer/Device/VulkanDevice.hpp>

#include <cstring>
#include <iostream>
#include <set>

namespace nvkg {

	vulkan_device_impl::vulkan_device_impl(Window* window) : window_{window} {
		NVKG_ASSERT(volkInitialize() == VK_SUCCESS, "Unable to initialise Volk!");

		create_instance();
		setup_debug_messenger();
		create_surface();
		pick_phys_device();
		create_logical_device();
		create_command_pool();
	}

	vulkan_device_impl::vulkan_device_impl() {}

	vulkan_device_impl::~vulkan_device_impl() {
		vkDestroyCommandPool(device_, command_pool_, nullptr);
		vkDestroyDevice(device_, nullptr);

		if (enable_validation_layers) {
			DebugUtilsMessenger::DestroyMessenger(instance_, debug_messenger_, nullptr);
		}

		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		vkDestroyInstance(instance_, nullptr);
	}

	void vulkan_device_impl::create_instance() {
		if (enable_validation_layers) {
			NVKG_ASSERT(Extensions::CheckValidationLayerSupport(validation_layers.data(), validation_layers.size()),
				"Validation Layers are not supported!");
		}
		
		// Specify general app information.
		VkApplicationInfo appInfo {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "nvkg";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get all extensions required by our windowing system. 
		auto extensions = Extensions::GetRequiredExtensions(enable_validation_layers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enable_validation_layers) {
			// Only add the ability to report on validation layers if the feature is enabled.
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			createInfo.ppEnabledLayerNames = validation_layers.data();

			DebugUtilsMessenger::PopulateCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		NVKG_ASSERT(vkCreateInstance(&createInfo, nullptr, OUT &instance_) == VK_SUCCESS, 
			"Unable to create Vulkan Instance!");

		Extensions::HasGflwRequiredInstanceExtensions(enable_validation_layers);

		volkLoadInstance(instance_);
	}

	void vulkan_device_impl::create_surface() { window_->init_window_surface(instance_, OUT &surface_); }

	void vulkan_device_impl::pick_phys_device() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, OUT &deviceCount, nullptr);

		NVKG_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan Support!");

        logger::debug(logger::Level::Info) << "Device count: " << deviceCount;

		VkPhysicalDevice devices[deviceCount];
		vkEnumeratePhysicalDevices(instance_, &deviceCount, OUT devices);

		for (size_t i = 0; i < deviceCount; i++) {
			VkPhysicalDevice device = devices[i];
			if (PhysicalDevice::IsSuitable(device, surface_, device_extensions.data(), device_extensions.size())) {
				physical_device_ = device;
				break;
			}
		}

		NVKG_ASSERT(physical_device_ != VK_NULL_HANDLE, "Failed to find a suitable GPU!");

		vkGetPhysicalDeviceProperties(physical_device_, OUT &properties);
        logger::debug(logger::Level::Info) << "physical device: " << properties.deviceName;
        logger::debug(logger::Level::Info) << "GPU has a minumum buffer alignment of " << properties.limits.minUniformBufferOffsetAlignment;
	}

	void vulkan_device_impl::create_logical_device() {
		QueueFamilyIndices::QueueFamilyIndices indices = QueueFamilyIndices::find_queue_families(physical_device_, surface_);

		std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family_, indices.present_family_};
		VkDeviceQueueCreateInfo queueCreateInfos[uniqueQueueFamilies.size()];

		float queuePriority = 1.0f;

		size_t index = 0;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos[index] = queueCreateInfo;
			index++;
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(uniqueQueueFamilies.size());
		createInfo.pQueueCreateInfos = queueCreateInfos;

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		createInfo.ppEnabledExtensionNames = device_extensions.data();

		if (enable_validation_layers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			createInfo.ppEnabledLayerNames = validation_layers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		NVKG_ASSERT(vkCreateDevice(physical_device_, &createInfo, nullptr, OUT &device_) == VK_SUCCESS, 
			"failed to create logical device!");

		vkGetDeviceQueue(device_, indices.graphics_family_, 0, OUT &graphics_queue_);
		vkGetDeviceQueue(device_, indices.compute_family, 0, OUT &compute_queue_);
		vkGetDeviceQueue(device_, indices.present_family_, 0, OUT &present_queue_);

		volkLoadDevice(device_);
	}

	void vulkan_device_impl::create_command_pool() {
		QueueFamilyIndices::QueueFamilyIndices queueFamilyIndices = find_phys_queue_families();

		VkCommandPoolCreateInfo poolInfo = initializers::command_pool_create_info();
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family_;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		NVKG_ASSERT(vkCreateCommandPool(device_, &poolInfo, nullptr, OUT &command_pool_) == VK_SUCCESS, "Failed to create command pool!");
	}

	void vulkan_device_impl::setup_debug_messenger() {
		if (!enable_validation_layers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		DebugUtilsMessenger::PopulateCreateInfo(OUT createInfo);

		NVKG_ASSERT(DebugUtilsMessenger::CreateMessenger(instance_, &createInfo, nullptr, OUT &debug_messenger_) == VK_SUCCESS, 
			"Failed to create DebugUtilsMessenger!");
	}

	VkFormat vulkan_device_impl::find_supported_format( const VkFormat* candidates, size_t formatCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (size_t i = 0; i < formatCount; i++) {
			VkFormat format = candidates[i];

			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physical_device_, format, OUT &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		NVKG_ASSERT(false, "Failed to find a supported format!");
	}

	uint32_t vulkan_device_impl::find_mem_type(uint32_t type_bits, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((type_bits & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{ return i; }
		}

		NVKG_ASSERT(false, "Failed to find suitable memory type!");
	}

	VkCommandBuffer vulkan_device_impl::begin_single_time_commands() {
		VkCommandBufferAllocateInfo allocInfo = initializers::command_buffer_allocate_info(command_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &allocInfo, OUT &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = initializers::command_buffer_begin_info();
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(OUT commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void vulkan_device_impl::end_single_time_commands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphics_queue_, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue_);

		vkFreeCommandBuffers(device_, command_pool_, 1, OUT &commandBuffer);
	}

	void vulkan_device_impl::cpy_buf(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = begin_single_time_commands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		end_single_time_commands(commandBuffer);
	}

	void vulkan_device_impl::cpy_buf_to_img(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
		VkCommandBuffer commandBuffer = begin_single_time_commands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
		end_single_time_commands(commandBuffer);
	}

	void vulkan_device_impl::create_img_with_info(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
		NVKG_ASSERT(vkCreateImage(device_, &imageInfo, nullptr, OUT &image) == VK_SUCCESS, 
			"Failed to create FrameImages!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, OUT &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_mem_type(memRequirements.memoryTypeBits, properties);

		NVKG_ASSERT(vkAllocateMemory(device_, &allocInfo, nullptr, OUT &imageMemory) == VK_SUCCESS,
				"Failed to allocate image memory!");

		NVKG_ASSERT(vkBindImageMemory(device_, OUT image, imageMemory, 0) == VK_SUCCESS,
				"Failed to bind image memory!");
	}

	vulkan_device_impl& device(Window* window) {
		static auto device_new_ = vulkan_device_impl(window);
		return device_new_;
	}
}
