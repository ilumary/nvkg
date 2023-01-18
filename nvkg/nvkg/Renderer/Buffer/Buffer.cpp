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

		NVKG_ASSERT(vkCreateBuffer(device().device(), &bufferInfo, nullptr, OUT &buffer) == VK_SUCCESS,
			"failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device().device(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device().find_mem_type(memRequirements.memoryTypeBits, properties);

		NVKG_ASSERT(vkAllocateMemory(device().device(), &allocInfo, nullptr, OUT &bufferMemory) == VK_SUCCESS,
			"failed to allocate vertex buffer memory!");

  		vkBindBufferMemory(device().device(), buffer, bufferMemory, 0);
    };

    void copy_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData, VkDeviceSize offset) {
        void* data;
        vkMapMemory(device().device(), dstBuffer.bufferMemory, offset, size, 0, &data);
        memcpy(data, bufferData, size);
        vkUnmapMemory(device().device(), dstBuffer.bufferMemory);
    }

    void append_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData) {
        void* data;
        vkMapMemory(device().device(), dstBuffer.bufferMemory, dstBuffer.size, size, 0, &data);
        memcpy(data, bufferData, size);
        vkUnmapMemory(device().device(), dstBuffer.bufferMemory);

        dstBuffer.size = dstBuffer.size + size;
    }

    void copy_buffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size) {
        device().cpy_buf(srcBuffer, dstBuffer, size);
    }

    void destroy_buffer(Buffer& buffer) {
        if (buffer.buffer != VK_NULL_HANDLE) vkDestroyBuffer(device().device(), buffer.buffer, nullptr);
        if (buffer.bufferMemory != VK_NULL_HANDLE) vkFreeMemory(device().device(), buffer.bufferMemory, nullptr);
    }

    size_t pad_uniform_buffer_size(size_t originalSize) {
        size_t minUboAlignment = device().get_device_alignment();
        size_t alignedSize = originalSize;
        if (minUboAlignment > 0) {
            alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        return alignedSize;
    }
}