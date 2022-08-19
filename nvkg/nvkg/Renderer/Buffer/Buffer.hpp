#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>

namespace nvkg::Buffer {

    struct Buffer {
        VkBuffer buffer {VK_NULL_HANDLE};
        VkDeviceMemory bufferMemory {VK_NULL_HANDLE};
        uint64_t size = 0;
    };

    /**
     * Creates a memory buffer for transferring data to our GPU. Allocates resulting data to the 'buffer'
     * and 'bufferMemory' variables respectively. 
     * 
     * @param size - specifies the size of the buffer.
     * @param usage - specifies what the buffer will be used for (i.e: vertex definitions).
     * @param properties - specifies the the properties the buffer should have.
     * @param buffer - the buffer that the function should write data to.
     * @param bufferMemory - the buffer memory data the function should write data to.
     **/
    void create_buffer(
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        VkBuffer &buffer,
		VkDeviceMemory &bufferMemory);

    /**
     * Returns a bitmask value representing the memory type required to allocate GPU memory.
     * Different rendering operations require different memory types. These are typically
     * differentiated using a bitmask. This functions throws an exception if the required 
     * memory type is unavailable.
     * @param typeFilter - The bitmask for the required memory type
     * @param properties - The available device memory properties. Used to query for available memory. 
     **/
    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
     * Copies data into a buffer. Data can be any type as specified by 
     * the templated type. 
     * 
     * @param dstBuffer - the buffer which data should be copied to.
     * @param size - the size of the buffer
     * @param bufferData - the data being copied into the buffer
     **/
    void copy_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData, VkDeviceSize offset = 0);

    void append_data(Buffer& dstBuffer, VkDeviceSize size, const void* bufferData);

    /**
     * Copies a buffer. This is generally used when copying the values within a buffer into
     * another (such as when setting up a staging buffer between the CPU and GPU).
     * 
     * @param srcBuffer - the buffer being copied.
     * @param dstBuffer - the buffer where the data is being copied into.
     * @param size - the expected size of the resulting buffer.
     **/
    void copy_buffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

    /**
     * Destroys a buffer struct and releases memory back to the device. 
     * 
     * @param buffer - the buffer to be destroyed.
     **/
    void destroy_buffer(Buffer& buffer);

    size_t pad_uniform_buffer_size(size_t originalSize);
}
