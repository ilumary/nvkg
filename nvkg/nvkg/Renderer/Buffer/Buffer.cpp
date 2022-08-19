#include <nvkg/Renderer/Buffer/Buffer.hpp>

namespace nvkg::Buffer {

    void create_buffer(
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        VkBuffer &buffer,
		VkDeviceMemory &bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto deviceInstance = VulkanDevice::get_device_instance();
        VkDevice device = deviceInstance->device();

		NVKG_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, OUT &buffer) == VK_SUCCESS,
			"failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = deviceInstance->find_mem_type(memRequirements.memoryTypeBits, properties);

		NVKG_ASSERT (vkAllocateMemory(device, &allocInfo, nullptr, OUT &bufferMemory) == VK_SUCCESS,
			"failed to allocate vertex buffer memory!");

  		vkBindBufferMemory(device, buffer, bufferMemory, 0);
    };

    void copy_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData, VkDeviceSize offset) {
        auto device = VulkanDevice::get_device_instance()->device();

        void* data;
        vkMapMemory(device, dstBuffer.bufferMemory, offset, size, 0, &data);
        memcpy(data, bufferData, size);
        vkUnmapMemory(device, dstBuffer.bufferMemory);
    }

    void append_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData) {
        auto device = VulkanDevice::get_device_instance()->device();

        void* data;
        vkMapMemory(device, dstBuffer.bufferMemory, dstBuffer.size, size, 0, &data);
        memcpy(data, bufferData, size);
        vkUnmapMemory(device, dstBuffer.bufferMemory);

        dstBuffer.size = dstBuffer.size + size;
    }

    void copy_buffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size) {
        VulkanDevice::get_device_instance()->cpy_buf(srcBuffer, dstBuffer, size);
    }

    void destroy_buffer(Buffer& buffer) {
        VkDevice device = VulkanDevice::get_device_instance()->device();
        if (buffer.buffer != VK_NULL_HANDLE) vkDestroyBuffer(device, buffer.buffer, nullptr);
        if (buffer.bufferMemory != VK_NULL_HANDLE) vkFreeMemory(device, buffer.bufferMemory, nullptr);
    }

    size_t pad_uniform_buffer_size(size_t originalSize) {
        size_t minUboAlignment = VulkanDevice::get_device_instance()->get_device_alignment();
        size_t alignedSize = originalSize;
        if (minUboAlignment > 0) {
            alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        return alignedSize;
    }
}