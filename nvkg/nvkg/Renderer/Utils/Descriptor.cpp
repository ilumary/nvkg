#include <nvkg/Renderer/Utils/Descriptor.hpp>
#include <iostream>

namespace nvkg::Utils {

    VkDescriptorSetLayoutBinding Descriptor::CreateLayoutBinding(uint32_t binding, uint32_t count, uint32_t type, 
                                                                    VkShaderStageFlags stageFlags, const VkSampler* sampler) {
        return {binding, (VkDescriptorType)type, count, stageFlags, sampler};
    }

    bool Descriptor::CreateLayout(VkDevice device, VkDescriptorSetLayout& layout, VkDescriptorSetLayoutBinding* bindings, 
                                    uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags, const void* pNext) {
        auto createInfo = CreateLayoutInfo(bindings, bindingCount, flags, pNext);

        return vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout) == VK_SUCCESS;
    }

    VkDescriptorSetLayoutCreateInfo Descriptor::CreateLayoutInfo(VkDescriptorSetLayoutBinding* bindings, uint32_t bindingCount, 
                                                                    VkDescriptorSetLayoutCreateFlags flags, const void* pNext) {
        return {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            pNext,
            flags, 
            bindingCount, 
            bindings
        };
    }

    void Descriptor::AllocateSets(VkDevice device, VkDescriptorSet* descriptorSets, VkDescriptorPool descriptorPool, 
                                    uint32_t setCount, const VkDescriptorSetLayout* layouts, const void* pNext) {
        auto allocateInfo = CreateAllocateInfo(descriptorPool, setCount, layouts, pNext);
        
        vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets);
    }

    VkDescriptorSetAllocateInfo Descriptor::CreateAllocateInfo(VkDescriptorPool pool, uint32_t setCount, 
                                                                const VkDescriptorSetLayout* layouts, const void* pNext) { 
        return {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            pNext, 
            pool, 
            setCount, 
            layouts
        };
    }

    VkDescriptorBufferInfo Descriptor::CreateBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size) {
        return { buffer, offset, size };
    }

    VkDescriptorImageInfo Descriptor::CreateImageInfo(VkSampler sampler, VkImageView view, VkImageLayout layout) {
        return { sampler, view, layout };
    }

    VkWriteDescriptorSet Descriptor::CreateBufferWriteSet(
        uint32_t dstBinding, 
        VkDescriptorSet& dstSet, 
        uint32_t descriptorCount, 
        VkDescriptorType type, 
        const VkDescriptorBufferInfo* bufferInfo
    ) {
        VkWriteDescriptorSet w = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dstSet,
            .dstBinding = dstBinding,
            .dstArrayElement = 0,
            .descriptorCount = descriptorCount,
            .descriptorType = type,
            .pBufferInfo = bufferInfo,
        };

        return w;
    }

    VkWriteDescriptorSet Descriptor::CreateImageWriteSet(
        uint32_t dstBinding, 
        VkDescriptorSet& dstSet, 
        uint32_t descriptorCount, 
        VkDescriptorType type, 
        const VkDescriptorImageInfo* imageInfo
    ) {
        VkWriteDescriptorSet w = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dstSet,
            .dstBinding = dstBinding,
            .dstArrayElement = 0,
            .descriptorCount = descriptorCount,
            .descriptorType = type,
            .pImageInfo = imageInfo,
        };

        return w;
    }

    void Descriptor::WriteSets(VkDevice device, const VkWriteDescriptorSet* sets, uint32_t setCount, const VkCopyDescriptorSet* copies, uint32_t copyCount) {
        vkUpdateDescriptorSets(device, setCount, sets, copyCount, copies);
    }
}