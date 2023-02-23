#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>

#include <unordered_map>

class Image {
    public:
        static VkImageView create_image_view(VkDevice device,
                                        VkImage image,
                                        VkImageViewType viewType,
                                        VkFormat format,
                                        VkComponentMapping componentMapping,
                                        VkImageSubresourceRange subResourceRange,
                                        const void* pNext = nullptr,
                                        VkImageViewCreateFlags createFlags = 0);

        static VkImageCreateInfo create_image_create_info(VkImageType type,
                                                    VkFormat format,
                                                    uint32_t imageWidth,
                                                    uint32_t imageHeight,
                                                    uint32_t imageDepth,
                                                    uint32_t mipLevels,
                                                    uint32_t arrayLevels,
                                                    VkSampleCountFlagBits sampleFlags,
                                                    VkImageTiling tiling,
                                                    VkImageUsageFlags usage,
                                                    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                                    uint32_t queueFamilyCount = 0,
                                                    const uint32_t* pQueueFamilyIndices = 0,
                                                    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VkImageCreateFlags flags = 0,
                                                    const void* pNext = nullptr);
    private:
        static VkImageViewCreateInfo create_image_view_create_info(VkImage image,
                                                            VkImageViewType viewType,
                                                            VkFormat format,
                                                            VkComponentMapping componentMapping,
                                                            VkImageSubresourceRange subResourceRange,
                                                            const void* pNext = nullptr,
                                                            VkImageViewCreateFlags createFlags = 0);
};

namespace nvkg {

    struct FormatInfo {
        uint32_t block_size;
        VkExtent3D block_extent;
    };

    class VulkanImage {
    public:

        VkImage image;
        VkFormat format;
        VkImageAspectFlags aspect_flags;
        VkImageType type;
        VkSampleCountFlagBits sample_count;
        VkImageLayout initial_layout;
        uint32_t mip_levels, array_layers;
        int width, height, depth;

        VulkanImage();
        ~VulkanImage();

        void create(VkExtent3D extent, VkFormat format, VkImageType type, VkImageCreateFlags flags,
                        VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t array_layers,
                        VkImageLayout initial_layout, VkSampleCountFlagBits sample_count);

        void update_and_transfer(void *data, VkDeviceSize size_in_bytes);


    private:
        VkBuffer staging_buffer_ = VK_NULL_HANDLE; // freed after staging
		VkDeviceMemory staging_memory_ = VK_NULL_HANDLE; // freed after staging
		VkDeviceMemory image_memory_ = VK_NULL_HANDLE;

        std::unordered_map<VkFormat, FormatInfo> format_info_table_ = {
            { VK_FORMAT_R8G8B8A8_UNORM, { 4, { 1, 1, 1 } } },
            { VK_FORMAT_R32G32_SFLOAT, { 8, { 1, 1, 1 } } },
            { VK_FORMAT_R32G32B32A32_SFLOAT, { 16, { 1, 1, 1 } } },
            { VK_FORMAT_BC3_UNORM_BLOCK, { 16, { 4, 4, 1 } } },
            { VK_FORMAT_R8_UNORM, { 1, { 1, 1, 1 } } },
            { VK_FORMAT_R8G8B8_UNORM, { 3, { 1, 1, 1 } } },
        };

        std::unordered_map<VkPipelineStageFlags, VkQueueFlags> pipeline_stage_queue_support_lut_ = {
            { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT},
            { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT },
            { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_QUEUE_COMPUTE_BIT },
            { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT },
            { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT },
            { VK_PIPELINE_STAGE_HOST_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT },
            { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT },
            { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT },
        };

        void allocate_transfer_mem(VkDeviceSize size_in_bytes);

        void transform_img_layout(VkCommandBuffer command_buffer, VkImage image, VkImageSubresourceRange subresource_range,
                        VkImageLayout old_layout, VkImageLayout new_layout);

        void alloc_mem(VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageLayout initial_layout,
                            VkSampleCountFlagBits sample_count, VkMemoryPropertyFlags memory_properties, VkImage &image,
                            VkDeviceMemory &memory);
        
        VkImageMemoryBarrier det_access_masks(VkImage image, VkImageSubresourceRange subresource_range,
                                                  VkImageLayout old_layout, VkImageLayout new_layout);

        static VkPipelineStageFlags determine_pipeline_stg_flg(VkAccessFlags access_flags) {
            // https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#synchronization-access-types-supported
            switch (access_flags) {
                case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:          return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

                case VK_ACCESS_INDEX_READ_BIT:
                case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:          return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

                case VK_ACCESS_SHADER_READ_BIT:
                case VK_ACCESS_SHADER_WRITE_BIT:
                case VK_ACCESS_UNIFORM_READ_BIT:                   return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                                                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

                case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:          return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
                case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:          return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
                case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                case VK_ACCESS_TRANSFER_READ_BIT:
                case VK_ACCESS_TRANSFER_WRITE_BIT:                  return VK_PIPELINE_STAGE_TRANSFER_BIT;

                case VK_ACCESS_HOST_READ_BIT:
                case VK_ACCESS_HOST_WRITE_BIT:                      return VK_PIPELINE_STAGE_HOST_BIT;

                default:                                            return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }
        }
    };

    class VulkanImageView {
        public:
            VkImageView image_view;
            VkImageViewType type;

            VulkanImageView();
            ~VulkanImageView();

            void create(VulkanImage *image, VkImageViewType image_view_type, uint32_t base_mip_level);

            void cleanup();

        private:
            VulkanImage *image_;
    };

    class VulkanSampler {
        public:
            VkSampler sampler;
            VkFilter mag_fltr, min_fltr;
            VkSamplerAddressMode u_add_m, v_add_m, w_add_m;
            VkSamplerMipmapMode mipmap_mode;
            bool uses_anisotropy, uses_unnormalized_coordinates;
            float mip_lod_bias, min_lod, max_lod, max_anisotropy;

            VulkanSampler();
            ~VulkanSampler();

            void create(VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode u, VkSamplerAddressMode v,
                        VkSamplerAddressMode w, bool enable_anisotropy, float max_anisotropy, VkSamplerMipmapMode mipmap_mode,
                        float mip_lod_bias, float min_lod, float max_lod, bool use_unnormalized_coordinates);

            void cleanup();
	};

}



