#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/Image/Utils/Image.hpp>

#include <vector>

namespace nvkg {

    struct SampledTexture {
        VulkanImage image{};
        VulkanImageView image_view{};
        VulkanSampler sampler{};
    };

    //TODO add option for global texture path
    class TextureManager {
        public:

            static SampledTexture* load_2d_img(std::string file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool create_mip_levels = true);

        private:

            /// @brief stores raw texture data on host memory
            static std::unordered_map<uint32_t, unsigned char *> m_ldr_texture_array_data_cache_;

            //basic texture creation from pixel data
            static SampledTexture* load_texture(void *data, VkDeviceSize size_in_bytes, VkExtent3D extent, VkFormat format,
                                VkImageCreateFlags flags, uint32_t mip_levels, uint32_t array_layers, VkImageViewType image_view_type);
    };

}