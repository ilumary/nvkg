#include <nvkg/Renderer/Texture/TextureManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace nvkg {

    TextureManager::TextureManager() {}
    TextureManager::~TextureManager() {}

    void TextureManager::init(VulkanDevice* device) {
        logger::debug(logger::Level::Info) << "Initializing Texture Manager";
        this->device = device;
    }

    SampledTexture* TextureManager::load_texture(void *data, VkDeviceSize size_in_bytes, VkExtent3D extent, VkFormat format,
                                    VkImageCreateFlags flags, uint32_t mip_levels, uint32_t array_layers, VkImageViewType image_view_type) {
        SampledTexture *texture = new SampledTexture();

        texture->image = new VulkanImage();
        texture->image->create(device, extent, format, VK_IMAGE_TYPE_2D, flags, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels,
                            array_layers, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_SAMPLE_COUNT_1_BIT);
        texture->image->update_and_transfer(data, size_in_bytes);
        logger::debug() << "Created Vulkan Image";

        texture->image_view = new VulkanImageView();
        texture->image_view->create(device, texture->image, image_view_type, 0);
        logger::debug() << "Created Vulkan Image View";

        texture->sampler = new VulkanSampler();
        texture->sampler->create(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true, 16, VK_SAMPLER_MIPMAP_MODE_LINEAR,
            0.f, 0.f, float(mip_levels - 1), false);
        logger::debug() << "Created Vulkan Image Sampler";

        return texture;
    }

    SampledTexture* TextureManager::load_2d_img(std::string file, VkFormat format, bool create_mip_levels) {
        int stb_format = (format == VK_FORMAT_R8G8B8A8_UNORM) ? STBI_rgb_alpha : 0; // todo: figure out how other formats play with stb
        int width, height, channels;

        // loads the image into a 1d array w/ 4 byte channel elements.
        unsigned char *texels = stbi_load(file.c_str(), &width, &height, &channels, stb_format);

        logger::debug() << "Loaded image file " << file << " with size " << (width * height * channels);

        if (!texels) {
            NVKG_ASSERT(false, "Could not load texture at location: " + file);
            return loaded_textures_[file];
        }

        m_ldr_texture_array_data_cache[file] = texels;

        uint32_t size = width * height * channels;
        VkExtent3D extent = {};
        extent.width = static_cast<uint32_t>(width);
        extent.height = static_cast<uint32_t>(height);
        extent.depth = 1;
        uint32_t mip_levels = (create_mip_levels) ? std::floor(std::log2(std::max(extent.width, extent.height))) + 1 : 1;

        loaded_textures_[file] = load_texture(texels, size, extent, format, 0, 1, 1, VK_IMAGE_VIEW_TYPE_2D);
        return loaded_textures_[file];
    }

}
