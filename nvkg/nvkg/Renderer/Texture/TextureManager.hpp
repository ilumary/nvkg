#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/Image/Utils/Image.hpp>

#include <vector>

namespace nvkg {

    struct SampledTexture {
        VulkanImage *image = nullptr;
        VulkanImageView *image_view = nullptr;
        VulkanSampler *sampler = nullptr;
    };

    //TODO add option for global texture path
    class TextureManager {
        public:
            TextureManager();
            ~TextureManager();

            void init(VulkanDevice* device);

            SampledTexture* load_2d_img(std::string file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool create_mip_levels = true);

        private:

            VulkanDevice *device;

            //stores constructed textures in current use
            std::unordered_map<std::string, SampledTexture*> loaded_textures_;

            //stores raw texture data on host memory
            std::unordered_map<std::string, unsigned char *> m_ldr_texture_array_data_cache;

            //basic texture creation from pixel data
            SampledTexture* load_texture(void *data, VkDeviceSize size_in_bytes, VkExtent3D extent, VkFormat format,
                                VkImageCreateFlags flags, uint32_t mip_levels, uint32_t array_layers, VkImageViewType image_view_type);
    };

}