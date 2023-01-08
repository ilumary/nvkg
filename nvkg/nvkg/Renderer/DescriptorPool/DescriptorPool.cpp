#include <nvkg/Renderer/DescriptorPool/DescriptorPool.hpp>

namespace nvkg {
    VkDescriptorPool DescriptorPool::descriptor_pool {VK_NULL_HANDLE};
    std::vector<VkDescriptorPoolSize> DescriptorPool::sizes;

    void DescriptorPool::add_pool_size(const VkDescriptorType type, const uint32_t size) {
        sizes.push_back({type, size});
    }

    void DescriptorPool::build_pool() {
        if (descriptor_pool == VK_NULL_HANDLE) {
            VkDescriptorPoolCreateInfo poolCreateInfo {};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.flags = 0;
            poolCreateInfo.maxSets = MAX_DESCRIPTOR_POOL_SIZES;
            poolCreateInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
            poolCreateInfo.pPoolSizes = sizes.data();

            NVKG_ASSERT(vkCreateDescriptorPool(device().device(), &poolCreateInfo, nullptr, &descriptor_pool) == VK_SUCCESS,
                "Unable to create descriptor pool!");
        }
    }

    void DescriptorPool::destroy_pool() {
        if (descriptor_pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device().device(), descriptor_pool, nullptr);
        }
    }
}