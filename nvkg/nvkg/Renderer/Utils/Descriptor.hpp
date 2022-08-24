#pragma once

#include <volk/volk.h>
#include <cstdint>

#include <nvkg/Renderer/Core.hpp>

// from Sasha Willems initialisers https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanInitializers.hpp
namespace nvkg::descriptors {

    inline VkDescriptorSetLayoutBinding descriptor_set_layout_binding (
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t binding,
            uint32_t descriptorCount = 1)
    {
            VkDescriptorSetLayoutBinding setLayoutBinding {
                .descriptorType = type,
                .stageFlags = stageFlags,
                .binding = binding,
                .descriptorCount = descriptorCount,
            };
            
            return setLayoutBinding;
    }

    inline VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(
			const VkDescriptorSetLayoutBinding* pBindings,
			uint32_t bindingCount)
    {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pBindings = pBindings,
                .bindingCount = bindingCount,
            };
			
			return descriptorSetLayoutCreateInfo;
    }

    inline VkDescriptorSetAllocateInfo descriptor_set_allocate_info(
			VkDescriptorPool descriptorPool,
			const VkDescriptorSetLayout* pSetLayouts,
			uint32_t descriptorSetCount)
    {
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = descriptorPool,
                .pSetLayouts = pSetLayouts,
                .descriptorSetCount = descriptorSetCount,
            };
			
			return descriptorSetAllocateInfo;
    }

    inline VkWriteDescriptorSet write_descriptor_set(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount = 1)
    {
			VkWriteDescriptorSet writeDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = dstSet,
                .descriptorType = type,
                .dstBinding = binding,
                .pBufferInfo = bufferInfo,
                .descriptorCount = descriptorCount,
            };
			
			return writeDescriptorSet;
    }

		inline VkWriteDescriptorSet write_descriptor_set(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo *imageInfo,
			uint32_t descriptorCount = 1)
    {
			VkWriteDescriptorSet writeDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = dstSet,
                .descriptorType = type,
                .dstBinding = binding,
                .pImageInfo = imageInfo,
                .descriptorCount = descriptorCount,
            };
			
			return writeDescriptorSet;
    }

}
