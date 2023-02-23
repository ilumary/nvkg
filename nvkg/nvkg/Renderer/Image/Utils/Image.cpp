#include <nvkg/Renderer/Image/Utils/Image.hpp>

VkImageView Image::create_image_view(VkDevice device,
                                   VkImage image,
                                   VkImageViewType viewType,
                                   VkFormat format,
                                   VkComponentMapping componentMapping,
                                   VkImageSubresourceRange subResourceRange,
                                   const void *pNext,
                                   VkImageViewCreateFlags createFlags) {
    VkImageView imageView;
    VkImageViewCreateInfo createInfo = create_image_view_create_info(image,
                                                                 viewType,
                                                                 format,
                                                                 componentMapping,
                                                                 subResourceRange,
                                                                 pNext,
                                                                 createFlags);

    NVKG_ASSERT(vkCreateImageView(device, &createInfo, nullptr, OUT &imageView) == VK_SUCCESS,
                "Failed to create texture image view!");

    return imageView;
}

VkImageViewCreateInfo Image::create_image_view_create_info(VkImage image,
                                                       VkImageViewType viewType,
                                                       VkFormat format,
                                                       VkComponentMapping componentMapping,
                                                       VkImageSubresourceRange subResourceRange,
                                                       const void *pNext,
                                                       VkImageViewCreateFlags
                                                       createFlags) {
    return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            pNext,
            createFlags,
            image,
            viewType,
            format,
            componentMapping,
            subResourceRange
    };
}

VkImageCreateInfo Image::create_image_create_info(VkImageType type,
                                               VkFormat format,
                                               uint32_t imageWidth,
                                               uint32_t imageHeight,
                                               uint32_t imageDepth,
                                               uint32_t mipLevels,
                                               uint32_t arrayLevels,
                                               VkSampleCountFlagBits sampleFlags,
                                               VkImageTiling tiling,
                                               VkImageUsageFlags usage,
                                               VkSharingMode sharingMode,
                                               uint32_t queueFamilyCount,
                                               const uint32_t *pQueueFamilyIndices,
                                               VkImageLayout initialLayout,
                                               VkImageCreateFlags flags,
                                               const void *pNext) {
    return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            pNext,
            flags,
            type,
            format,
            {
                imageWidth,
                imageHeight,
                imageDepth
            },
            mipLevels,
            arrayLevels,
            sampleFlags,
            tiling,
            usage,
            sharingMode,
            queueFamilyCount,
            pQueueFamilyIndices,
            initialLayout
    };
}

namespace nvkg {

    VulkanImage::VulkanImage(){}

    VulkanImage::~VulkanImage() {
        vkDestroyImage(device().device(), image, nullptr);
        vkFreeMemory(device().device(), image_memory_, nullptr);
    }

    void VulkanImage::create(VkExtent3D extent, VkFormat format, VkImageType type, VkImageCreateFlags flags,
                        VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t array_layers,
                        VkImageLayout initial_layout, VkSampleCountFlagBits sample_count) {
        
		this->format = format;
        this->width = extent.width;
        this->height = extent.height;
        this->depth = extent.depth;
        this->type = type;
        this->aspect_flags = aspect_flags;
        this->mip_levels = mip_levels;
        this->array_layers = array_layers;
        this->sample_count = sample_count;
        this->initial_layout = initial_layout;

        alloc_mem(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            flags, initial_layout, sample_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory_);

    }

    void VulkanImage::alloc_mem(VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageLayout initial_layout,
                            VkSampleCountFlagBits sample_count, VkMemoryPropertyFlags memory_properties, VkImage &image,
                            VkDeviceMemory &memory) {
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.flags = flags;
        image_create_info.extent.width = static_cast<uint32_t>(this->width);
        image_create_info.extent.height = static_cast<uint32_t>(this->height);
        image_create_info.extent.depth = static_cast<uint32_t>(this->depth);
        image_create_info.imageType = this->type;
        image_create_info.mipLevels = this->mip_levels;
        image_create_info.arrayLayers = this->array_layers;
        image_create_info.format = this->format; // can be changed at a later time
        image_create_info.tiling = tiling;
        image_create_info.usage = usage;
        image_create_info.initialLayout = initial_layout;
        image_create_info.samples = sample_count;
		
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        /*image_create_info.queueFamilyIndexCount = 1;
        uint32_t queue_index = static_cast<uint32_t>(device().find_phys_queue_families().graphics_family_);
        image_create_info.pQueueFamilyIndices = &queue_index;*/
        
		NVKG_ASSERT(vkCreateImage(device().device(), &image_create_info, nullptr, &image) == VK_SUCCESS, "Error creating Image");
        
		// Determine requirements for memory (where it's allocated, type of memory, etc.)
		VkMemoryRequirements memory_requirements = {};
		vkGetImageMemoryRequirements(device().device(), image, &memory_requirements);
		auto memory_type = device().find_mem_type(memory_requirements.memoryTypeBits, memory_properties);

		// Allocate and bind buffer memory.
		VkMemoryAllocateInfo memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type,
        };

		vkAllocateMemory(device().device(), &memory_allocate_info, nullptr, &memory);
		vkBindImageMemory(device().device(), image, memory, 0);
    }

    void VulkanImage::update_and_transfer(void *data, VkDeviceSize size_in_bytes) {
        auto buf = device().begin_single_time_commands();

        VkImageSubresourceRange subresource_range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = this->mip_levels,
            .layerCount = this->array_layers,
        };

        transform_img_layout(buf, image, subresource_range, initial_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // move host data to transfer buffer
        allocate_transfer_mem(size_in_bytes);
        void *mapped_data;
        vkMapMemory(device().device(), staging_memory_, 0, size_in_bytes, 0, &mapped_data);
        memcpy(mapped_data, data, size_in_bytes);
        vkUnmapMemory(device().device(), staging_memory_);
        mapped_data = nullptr;

        const auto &format_info = format_info_table_.at(format);
		const uint32_t block_size = format_info.block_size;
		const uint32_t block_width = format_info.block_extent.width;
		const uint32_t block_height = format_info.block_extent.height;
		const uint32_t block_depth = format_info.block_extent.depth;

		// Copy mip levels from staging buffer
		std::vector<VkBufferImageCopy> buffer_copy_regions;
		uint32_t offset = 0;

		for (uint32_t layer = 0; layer < this->array_layers; layer++) {
			for (uint32_t level = 0; level < this->mip_levels; level++) {
				uint32_t image_width = (this->width >> level);
				uint32_t image_height = (this->height >> level);
				uint32_t block_count_x = (image_width + (block_width - 1)) / block_width;
				uint32_t block_count_y = (image_height + (block_height - 1)) / block_height;
				uint32_t block_count_z = (depth + (block_depth - 1)) / block_depth;

				VkBufferImageCopy buffer_copy_region = {};
                buffer_copy_region.imageSubresource.aspectMask = this->aspect_flags;
                buffer_copy_region.imageSubresource.mipLevel = level;
                buffer_copy_region.imageSubresource.baseArrayLayer = layer;
                buffer_copy_region.imageSubresource.layerCount = 1;
                buffer_copy_region.imageExtent.width = image_width;
                buffer_copy_region.imageExtent.height = image_height;
                buffer_copy_region.imageExtent.depth = static_cast<uint32_t>(depth);
                buffer_copy_region.bufferOffset = offset;
                

				buffer_copy_regions.push_back(buffer_copy_region);
				offset += block_count_x * block_count_y * block_count_z * block_size;
			}
		}

		vkCmdCopyBufferToImage(buf, staging_buffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(buffer_copy_regions.size()), buffer_copy_regions.data());

        transform_img_layout(buf, image, subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        device().end_single_time_commands(buf);

        vkDestroyBuffer(device().device(), staging_buffer_, nullptr);
        vkFreeMemory(device().device(), staging_memory_, nullptr);
    }

    void VulkanImage::transform_img_layout(VkCommandBuffer command_buffer, VkImage image, VkImageSubresourceRange subresource_range,
                        VkImageLayout old_layout, VkImageLayout new_layout) {
        VkImageMemoryBarrier memory_barrier = det_access_masks(image, subresource_range, old_layout, new_layout);
        VkPipelineStageFlags old_stage = determine_pipeline_stg_flg(memory_barrier.srcAccessMask);
        VkPipelineStageFlags new_stage = determine_pipeline_stg_flg(memory_barrier.dstAccessMask);
		vkCmdPipelineBarrier(command_buffer, old_stage, new_stage, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);
    }

    void VulkanImage::allocate_transfer_mem(VkDeviceSize size_in_bytes) {
        VkBufferCreateInfo buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = 0,
            .size = size_in_bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

		vkCreateBuffer(device().device(), &buffer_create_info, nullptr, &staging_buffer_);

		VkMemoryRequirements memory_requirements = {};
		vkGetBufferMemoryRequirements(device().device(), staging_buffer_, &memory_requirements);

        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = device().find_mem_type(memory_requirements.memoryTypeBits,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };
        
		vkAllocateMemory(device().device(), &memory_alloc_info, nullptr, &staging_memory_);
		vkBindBufferMemory(device().device(), staging_buffer_, staging_memory_, 0);
    }

    VkImageMemoryBarrier VulkanImage::det_access_masks(VkImage image, VkImageSubresourceRange subresource_range, VkImageLayout old_layout, VkImageLayout new_layout) {
        VkImageMemoryBarrier memory_barrier = {};
        memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memory_barrier.oldLayout = old_layout;
		memory_barrier.newLayout = new_layout;
        //memory_barrier.srcAccessMask = this->aspect_flags;
        //memory_barrier.dstAccessMask = this->aspect_flags;
		memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.image = image;
        memory_barrier.subresourceRange = subresource_range;

        // https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp#L94
        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (old_layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                memory_barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source 
                // Make sure any reads from the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
		switch (new_layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from and writes to the image have been finished
                memory_barrier.srcAccessMask = memory_barrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                memory_barrier.dstAccessMask = memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (memory_barrier.srcAccessMask == 0)
                	memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

                memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
        }

        return memory_barrier;
    }

    VulkanImageView::VulkanImageView() {}
    VulkanImageView::~VulkanImageView() {
        vkDestroyImageView(device().device(), image_view, nullptr);
    }

    void VulkanImageView::create(VulkanImage *image, VkImageViewType image_view_type, uint32_t base_mip_level) {
		image_ = image;
        type = image_view_type;

		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//image_view_create_info.flags = VK_NULL_HANDLE;
		image_view_create_info.image = image->image;
		image_view_create_info.viewType = image_view_type;
		image_view_create_info.format = image->format;

		// todo: make general
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		image_view_create_info.subresourceRange.aspectMask = image->aspect_flags;
        image_view_create_info.subresourceRange.baseMipLevel = base_mip_level;
		image_view_create_info.subresourceRange.levelCount = image->mip_levels;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = image->array_layers;

		vkCreateImageView(device().device(), &image_view_create_info, nullptr, &image_view);
	}


	void VulkanImageView::cleanup() {
        vkDestroyImageView(device().device(), image_view, nullptr);
	}

    VulkanSampler::VulkanSampler() {}
    
    VulkanSampler::~VulkanSampler() {
        vkDestroySampler(device().device(), sampler, nullptr);
    }

    void VulkanSampler::create(VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode u, VkSamplerAddressMode v,
                        VkSamplerAddressMode w, bool enable_anisotropy, float max_anisotropy, VkSamplerMipmapMode mipmap_mode,
                        float mip_lod_bias, float min_lod, float max_lod, bool use_unnormalized_coordinates) {
        
        NVKG_ASSERT(max_anisotropy <= 16, "VulkanSampler cannot have anisotropy higher than 16");
        if (use_unnormalized_coordinates) {
            NVKG_ASSERT(mag_filter == min_filter, "VulkanSampler must have mag and min filter equal when using unnormalized coordinates");
            NVKG_ASSERT(mipmap_mode == VK_SAMPLER_MIPMAP_MODE_NEAREST, "VulkanSampler must use VK_SAMPLER_MIPMAP_MODE_NEAREST when using unnormalized coordinates");
            NVKG_ASSERT(min_lod == 0 && max_lod == 0, "VulkanSampler's min and max lod must be 0 when using unnormalized coordinates");
            NVKG_ASSERT(u == v, "VulkanSampler's u and v addressing modes must be equal when using unnormalized coordinates");
            NVKG_ASSERT(u == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE || u == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                "VulkanSampler's address modes must have must use either VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER when using unnormalized coordinates");
            NVKG_ASSERT(!enable_anisotropy, "VulkanSampler doesn't support anisotropy when using unnormalized coordinates");
        }

        this->mag_fltr = mag_filter;
        this->min_fltr = min_filter;
        this->u_add_m = u;
        this->v_add_m = v;
        this->w_add_m = w;
        this->uses_anisotropy = enable_anisotropy;
        this->max_anisotropy = max_anisotropy;
        this->mipmap_mode = mipmap_mode;
        this->mip_lod_bias = mip_lod_bias;
        this->min_lod = min_lod;
        this->max_lod = max_lod;
        this->uses_unnormalized_coordinates = use_unnormalized_coordinates;

        VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = mag_filter;
		sampler_create_info.minFilter = min_filter;

        sampler_create_info.addressModeU = u;
		sampler_create_info.addressModeV = v;
		sampler_create_info.addressModeW = w;

		sampler_create_info.anisotropyEnable = enable_anisotropy;
		sampler_create_info.maxAnisotropy = max_anisotropy;

		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // black, white, transparent
		sampler_create_info.unnormalizedCoordinates = use_unnormalized_coordinates;

		sampler_create_info.mipmapMode = mipmap_mode;
		sampler_create_info.mipLodBias = mip_lod_bias;
		sampler_create_info.minLod = min_lod;
		sampler_create_info.maxLod = max_lod; // todo: figure out how lod works with these things

		vkCreateSampler(device().device(), &sampler_create_info, nullptr, &sampler);
    }

    void VulkanSampler::cleanup() {
        vkDestroySampler(device().device(), sampler, nullptr);
    }

}
