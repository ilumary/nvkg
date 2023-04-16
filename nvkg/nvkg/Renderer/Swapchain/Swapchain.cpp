#include <nvkg/Renderer/Swapchain/Swapchain.hpp>

namespace nvkg {
    SwapChain* SwapChain::instance_ = nullptr;

    SwapChain::SwapChain(VkExtent2D windowExtent) : window_extent_{windowExtent} {
        init();
    }

    SwapChain::SwapChain() {}

    SwapChain::~SwapChain() {
        clear_swapchain();
        clear_memory();
    }

    void SwapChain::set_window_extents(VkExtent2D windowExtent) {
        this->window_extent_ = windowExtent;
        init();
    }

    void SwapChain::clear_swapchain(bool isRecreated) {
        uint32_t imageCount = FrameImages::GetImageCount();

        swapchain_images_.DestroyFrameImages();

        if (!isRecreated && swapchain_ != nullptr) {
            vkDestroySwapchainKHR(device().device(), get_swapchain(), nullptr);
            swapchain_ = nullptr;
        }

        depth_images_.DestroyFrameImages();

        for (size_t i = 0; i < imageCount; i++) {
            vkDestroyFramebuffer(device().device(), swapchain_frame_buffers_[i], nullptr);
        }
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device().device(), render_finished_semaphores_[i], nullptr);
            vkDestroySemaphore(device().device(), image_available_semaphores_[i], nullptr);
            vkDestroyFence(device().device(), in_flight_fences_[i], nullptr);
        }
    }

    void SwapChain::clear_memory() {
        delete [] swapchain_frame_buffers_;
        
        delete [] images_in_flight_;

        swapchain_frame_buffers_ = nullptr;

        images_in_flight_ = nullptr;
    }

    void SwapChain::recreate_swapchain() {
        logger::debug() << "Re-creating Swapchain";
        clear_swapchain(true);
        init();
    }

    bool SwapChain::compare_swap_formats(VkFormat oldImageFormat, VkFormat oldDepthFormat) {
        return oldImageFormat == swapchain_image_format_ && oldDepthFormat == swapchain_depth_format_;
    }

    void SwapChain::init() {
        create_swapchain();
        create_image_views();
        create_renderpass();
        create_depth_resources();
        create_frame_buffers();
        create_sync_objects();

        if (instance_ == nullptr) instance_ = this;
    }

    void SwapChain::create_swapchain() {
        SwapChainSupportDetails::SwapChainSupportDetails details = device().get_swapchain_support();

        VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(details.formats.data(),
                                                                   static_cast<uint32_t>(details.formats.size()));

        VkPresentModeKHR presentMode = choose_present_mode(details.presentModes.data(),
                                                         static_cast<uint32_t>(details.presentModes.size()));

        VkExtent2D extent = choose_swap_extent(details.capabilities);

        logger::debug(logger::Level::Info) << "Extent: " << extent.width << "x" << extent.height;

        uint32_t imageCount = details.capabilities.minImageCount + 1;

        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
            imageCount = details.capabilities.maxImageCount;
        }   
        logger::debug(logger::Level::Info) << "FrameImages Count: " << imageCount;

        VkSwapchainCreateInfoKHR createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = device().surface();
        
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        QueueFamilyIndices::QueueFamilyIndices indices = device().find_phys_queue_families();
        uint32_t queueFamilyIndices[] = {indices.graphics_family_, indices.present_family_};

        if (indices.graphics_family_ != indices.present_family_) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = swapchain_ ? swapchain_ : VK_NULL_HANDLE;

        NVKG_ASSERT(vkCreateSwapchainKHR(device().device(), &createInfo, nullptr, OUT &swapchain_) == VK_SUCCESS,
                "Failed to create Swapchain!");

        swapchain_images_ = FrameImages(surfaceFormat.format);

        vkGetSwapchainImagesKHR(device().device(), swapchain_, OUT &imageCount, nullptr);

        FrameImages::SetImageCount(imageCount);

        vkGetSwapchainImagesKHR(device().device(), swapchain_, &imageCount, OUT swapchain_images_.GetImages());

        swapchain_extent_ = extent;
    }

    void SwapChain::create_image_views() {
        swapchain_images_.InitColorImageView2D();
    }

    void SwapChain::create_renderpass() {
        swapchain_image_format_ = get_swapchain_image_format();
        swapchain_depth_format_ = find_depth_format();
        RenderPass::Initialise(OUT renderpass,
                               RenderPass::CreateConfig()
                              .WithAttachment(Attachments::CreateColorAttachment(swapchain_image_format_))
                              .WithAttachment(Attachments::CreateDepthAttachment(swapchain_depth_format_))
                              .WithSubPass(Attachments::CreateSubPass()
                                            .WithColorReference(Attachments::CreateColorAttachmentReference(0))
                                            .WithDepthReference(Attachments::CreateDepthStencilAttachmentReference(1))
                                            .BuildGraphicsSubPass())
                              .WithDependency(Attachments::CreateSubPassDependency()
                                              .WithSrcSubPass(VK_SUBPASS_EXTERNAL)
                                              .WithDstSubPass(0)
                                              .WithSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                                                                        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
                                              .WithDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                                                                        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
                                              .WithSrcAccessMask(0)
                                              .WithDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                                                                        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
                                              .Build()));
    }

    void SwapChain::create_depth_resources() {
        VkExtent2D extent = get_swapchain_extent();

        depth_images_ = FrameImages(swapchain_depth_format_);

        depth_images_.InitDepthImageView2D(extent.width, extent.height, 1);
    }

    void SwapChain::create_frame_buffers() {
        uint32_t imageCount = FrameImages::GetImageCount();

        if (swapchain_frame_buffers_ == nullptr) swapchain_frame_buffers_ = new VkFramebuffer[imageCount];

        for(size_t i = 0; i < imageCount; i++) {
            uint32_t attachmentCount = 2;

            VkImageView attachments[] {swapchain_images_.GetImageView(i), depth_images_.GetImageView(i)};

            VkExtent2D swapChainExtent = get_swapchain_extent();

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = renderpass.get();
            frameBufferInfo.attachmentCount = attachmentCount;
            frameBufferInfo.pAttachments = attachments;
            frameBufferInfo.width = swapChainExtent.width;
            frameBufferInfo.height = swapChainExtent.height;
            frameBufferInfo.layers = 1;

            NVKG_ASSERT(vkCreateFramebuffer(device().device(),
                                            &frameBufferInfo, nullptr, OUT &swapchain_frame_buffers_[i]) == VK_SUCCESS,
                                            "Failed to create framebuffer");
        }
    }

    void SwapChain::create_sync_objects() {
        uint32_t imageCount = FrameImages::GetImageCount();
        
        if (image_available_semaphores_ == nullptr) image_available_semaphores_ = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
        if (render_finished_semaphores_ == nullptr) render_finished_semaphores_ = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
        if (in_flight_fences_ == nullptr) in_flight_fences_ = new VkFence[MAX_FRAMES_IN_FLIGHT];
        if (images_in_flight_ == nullptr) images_in_flight_ = new VkFence[imageCount];

        for (size_t i = 0; i < imageCount; i++) images_in_flight_[i] = VK_NULL_HANDLE;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            NVKG_ASSERT(
                vkCreateSemaphore(device().device(), &semaphoreInfo, nullptr, OUT &image_available_semaphores_[i]) == VK_SUCCESS &&
                vkCreateSemaphore(device().device(), &semaphoreInfo, nullptr, OUT &render_finished_semaphores_[i]) == VK_SUCCESS &&
                vkCreateFence(device().device(), &fenceInfo, nullptr, OUT &in_flight_fences_[i]) == VK_SUCCESS, 
                "Failed to create synchronization objects fora  frame!");
        }
    }

    VkResult SwapChain::acquire_next_image(uint32_t* imageIndex) {
        vkWaitForFences(
            device().device(), 
            1, 
            &in_flight_fences_[current_frame_], 
            VK_TRUE, 
            std::numeric_limits<uint64_t>::max());

        return vkAcquireNextImageKHR(
            device().device(),
            swapchain_, 
            std::numeric_limits<uint64_t>::max(),
            image_available_semaphores_[current_frame_],
            VK_NULL_HANDLE,
            imageIndex
        ); 
    }

    VkResult SwapChain::submit_command_buffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
        uint32_t index = *imageIndex;

        if (images_in_flight_[index] != VK_NULL_HANDLE) {
            vkWaitForFences(device().device(), 1, &images_in_flight_[index], VK_TRUE, UINT64_MAX);
        }

        images_in_flight_[index] = in_flight_fences_[current_frame_];

        VkSubmitInfo submitInfo{};
        
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {image_available_semaphores_[current_frame_]};

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {render_finished_semaphores_[current_frame_]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        vkResetFences(device().device(), 1, OUT &in_flight_fences_[current_frame_]);

        NVKG_ASSERT(vkQueueSubmit(device().graphics_queue(), 1, &submitInfo, OUT in_flight_fences_[current_frame_]) == VK_SUCCESS,
            "Failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] {swapchain_};

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        auto result = vkQueuePresentKHR(device().present_queue(), &presentInfo);

        current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    VkSurfaceFormatKHR SwapChain::choose_swap_surface_format(VkSurfaceFormatKHR* formats, size_t formatCount) {
        for (size_t i = 0; i < formatCount; i++) {
            VkSurfaceFormatKHR& availableFormat = formats[i];
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return availableFormat;
        }
        return formats[0];
    }

    VkPresentModeKHR SwapChain::choose_present_mode(VkPresentModeKHR* presentModes, size_t presentModeCount) {
        for (size_t i = 0; i < presentModeCount; i++) {
            VkPresentModeKHR& availablePresentMode = presentModes[i];

            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                logger::debug(logger::Level::Info) << "Present Mode: Mailbox";
                return availablePresentMode;
            }
        }

        logger::debug(logger::Level::Info) << "Present Mode: V-Sync";
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::choose_swap_extent(VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
        else {
            VkExtent2D actualExtent = window_extent_;
            actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width)
            );
            actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height)
            );
            return actualExtent;
        }
    }

    VkFormat SwapChain::find_depth_format() {
        VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
        return device().find_supported_format(
            formats,
            3,
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
