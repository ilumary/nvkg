#pragma once

#include <volk/volk.h>
#include <cstdint>

#include <nvkg/Renderer/Core.hpp>

namespace nvkg::Utils {
    // TODO document this
    class Descriptor {
        public: 

        struct DescriptorInfo {
            unsigned binding;
            std::string name;
            VkShaderStageFlagBits shader_stage;
            VkDescriptorType type;
        };

        enum Type
        {
            UNIFORM = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            STORAGE = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            UNIFORM_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            STORAGE_DYNAMIC = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
        };

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(
            uint32_t binding, 
            uint32_t count, 
            uint32_t type, 
            VkShaderStageFlags stageFlags,
            const VkSampler* sampler = nullptr
        );

        static bool CreateLayout(
            VkDevice device,
            VkDescriptorSetLayout& layout,
            VkDescriptorSetLayoutBinding* bindings, 
            uint32_t bindingCount, 
            VkDescriptorSetLayoutCreateFlags flags = 0,
            const void* pNext = nullptr
        );

        static VkDescriptorSetLayoutCreateInfo CreateLayoutInfo(
            VkDescriptorSetLayoutBinding* bindings, 
            uint32_t bindingCount, 
            VkDescriptorSetLayoutCreateFlags flags = 0,
            const void* pNext = nullptr
        );

        static void AllocateSets(
            VkDevice device, 
            VkDescriptorSet* descriptorSets,
            VkDescriptorPool descriptorPool, 
            uint32_t setCount, 
            const VkDescriptorSetLayout* layouts, 
            const void* pNext = nullptr
        );

        static VkDescriptorSetAllocateInfo CreateAllocateInfo(
            VkDescriptorPool pool, 
            uint32_t setCount, 
            const VkDescriptorSetLayout* layouts, 
            const void* pNext = nullptr
        );

        static VkDescriptorBufferInfo CreateBufferInfo(
            VkBuffer buffer, 
            VkDeviceSize offset, 
            VkDeviceSize size
        );

        static VkDescriptorImageInfo CreateImageInfo(
            VkSampler sampler,
            VkImageView view,
            VkImageLayout layout
        );

        static VkWriteDescriptorSet CreateBufferWriteSet(
            uint32_t dstBinding,
            VkDescriptorSet& dstSet,
            uint32_t descriptorCount,
            VkDescriptorType type, 
            const VkDescriptorBufferInfo* bufferInfo
        );

        static VkWriteDescriptorSet CreateImageWriteSet(
            uint32_t dstBinding, 
            VkDescriptorSet& dstSet, 
            uint32_t descriptorCount, 
            VkDescriptorType type, 
            const VkDescriptorImageInfo* imageInfo
        );

        static void WriteSets(
            VkDevice device, 
            const VkWriteDescriptorSet* sets, 
            uint32_t setCount,
            const VkCopyDescriptorSet* copies = nullptr,
            uint32_t copyCount = 0
        );

        private: 
    };
}
