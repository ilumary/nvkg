#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>

namespace nvkg {
    
    class DescriptorPool {
        public:
        
        static void add_pool_size(const VkDescriptorType type, const uint32_t size);
        static VkDescriptorPool& get_descr_pool() { return descriptor_pool; }
        
        static void build_pool();
        static void destroy_pool();

        private:

        static constexpr size_t MAX_DESCRIPTOR_POOL_SIZES = 10;
        
        static VkDescriptorPool descriptor_pool;
        static std::vector<VkDescriptorPoolSize> sizes;
    };
}