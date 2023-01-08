#include <nvkg/Renderer/Swapchain/Swapchain.hpp>

namespace nvkg {
    SwapChain* SwapChain::instance = nullptr;

    SwapChain::SwapChain(VkExtent2D windowExtent) : windowExtent{windowExtent} {
        Init();
    }

    SwapChain::SwapChain() {}

    SwapChain::~SwapChain() {
        ClearSwapChain();
        ClearMemory();
    }

    void SwapChain::SetWindowExtents(VkExtent2D windowExtent) {
        this->windowExtent = windowExtent;
        Init();
    }

    void SwapChain::ClearSwapChain(bool isRecreated) {
        uint32_t imageCount = FrameImages::GetImageCount();

        swapchainImages.DestroyFrameImages();

        if (!isRecreated && swapChain != nullptr) {
            vkDestroySwapchainKHR(device().device(), GetSwapChain(), nullptr);
            swapChain = nullptr;
        }

        depthImages.DestroyFrameImages();

        for (size_t i = 0; i < imageCount; i++) {
            vkDestroyFramebuffer(device().device(), swapChainFrameBuffers[i], nullptr);
        }
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device().device(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device().device(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device().device(), inFlightFences[i], nullptr);
        }
    }

    void SwapChain::ClearMemory() {
        delete [] swapChainFrameBuffers;
        
        delete [] imagesInFlight;

        swapChainFrameBuffers = nullptr;

        imagesInFlight = nullptr;
    }

    void SwapChain::RecreateSwapchain() {
        logger::debug() << "Re-creating Swapchain";
        ClearSwapChain(true);
        Init();
    }

    bool SwapChain::CompareSwapFormats(VkFormat oldImageFormat, VkFormat oldDepthFormat) {
        return oldImageFormat == swapChainImageFormat && oldDepthFormat == swapChainDepthFormat;
    }

    void SwapChain::Init() {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDepthResources();
        CreateFrameBuffers();
        CreateSyncObjects();

        if (instance == nullptr) instance = this;
    }

    void SwapChain::CreateSwapChain() {
        SwapChainSupportDetails::SwapChainSupportDetails details = device().get_swapchain_support();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats.data(),
                                                                   static_cast<uint32_t>(details.formats.size()));

        VkPresentModeKHR presentMode = ChoosePresentMode(details.presentModes.data(),
                                                         static_cast<uint32_t>(details.presentModes.size()));

        VkExtent2D extent = ChooseSwapExtent(details.capabilities);

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

        createInfo.oldSwapchain = swapChain ? swapChain : VK_NULL_HANDLE;

        NVKG_ASSERT(vkCreateSwapchainKHR(device().device(), &createInfo, nullptr, OUT &swapChain) == VK_SUCCESS,
                "Failed to create Swapchain!");

        swapchainImages = FrameImages(surfaceFormat.format);

        vkGetSwapchainImagesKHR(device().device(), swapChain, OUT &imageCount, nullptr);

        FrameImages::SetImageCount(imageCount);

        vkGetSwapchainImagesKHR(device().device(), swapChain, &imageCount, OUT swapchainImages.GetImages());

        swapChainExtent = extent;
    }

    void SwapChain::CreateImageViews() {
        swapchainImages.InitColorImageView2D();
    }

    void SwapChain::CreateRenderPass() {
        swapChainImageFormat = GetSwapChainImageFormat();
        swapChainDepthFormat = FindDepthFormat();
        RenderPass::Initialise(OUT renderPass,
                               RenderPass::CreateConfig()
                              .WithAttachment(Attachments::CreateColorAttachment(swapChainImageFormat))
                              .WithAttachment(Attachments::CreateDepthAttachment(swapChainDepthFormat))
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

    void SwapChain::CreateDepthResources() {
        VkExtent2D extent = GetSwapChainExtent();

        depthImages = FrameImages(swapChainDepthFormat);

        depthImages.InitDepthImageView2D(extent.width, extent.height, 1);
    }

    void SwapChain::CreateFrameBuffers() {
        uint32_t imageCount = FrameImages::GetImageCount();

        if (swapChainFrameBuffers == nullptr) swapChainFrameBuffers = new VkFramebuffer[imageCount];

        for(size_t i = 0; i < imageCount; i++) {
            uint32_t attachmentCount = 2;

            VkImageView attachments[] {swapchainImages.GetImageView(i), depthImages.GetImageView(i)};

            VkExtent2D swapChainExtent = GetSwapChainExtent();

            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.renderPass = renderPass.GetRenderPass();
            frameBufferInfo.attachmentCount = attachmentCount;
            frameBufferInfo.pAttachments = attachments;
            frameBufferInfo.width = swapChainExtent.width;
            frameBufferInfo.height = swapChainExtent.height;
            frameBufferInfo.layers = 1;

            NVKG_ASSERT(vkCreateFramebuffer(device().device(),
                                            &frameBufferInfo, nullptr, OUT &swapChainFrameBuffers[i]) == VK_SUCCESS,
                                            "Failed to create framebuffer");
        }
    }

    void SwapChain::CreateSyncObjects() {
        uint32_t imageCount = FrameImages::GetImageCount();
        
        if (imageAvailableSemaphores == nullptr) imageAvailableSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
        if (renderFinishedSemaphores == nullptr) renderFinishedSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
        if (inFlightFences == nullptr) inFlightFences = new VkFence[MAX_FRAMES_IN_FLIGHT];
        if (imagesInFlight == nullptr) imagesInFlight = new VkFence[imageCount];

        for (size_t i = 0; i < imageCount; i++) imagesInFlight[i] = VK_NULL_HANDLE;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            NVKG_ASSERT(
                vkCreateSemaphore(device().device(), &semaphoreInfo, nullptr, OUT &imageAvailableSemaphores[i]) == VK_SUCCESS &&
                vkCreateSemaphore(device().device(), &semaphoreInfo, nullptr, OUT &renderFinishedSemaphores[i]) == VK_SUCCESS &&
                vkCreateFence(device().device(), &fenceInfo, nullptr, OUT &inFlightFences[i]) == VK_SUCCESS, 
                "Failed to create synchronization objects fora  frame!");
        }
    }

    VkResult SwapChain::AcquireNextImage(uint32_t* imageIndex) {
        vkWaitForFences(
            device().device(), 
            1, 
            &inFlightFences[currentFrame], 
            VK_TRUE, 
            std::numeric_limits<uint64_t>::max());

        return vkAcquireNextImageKHR(
            device().device(),
            swapChain, 
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE,
            imageIndex
        ); 
    }

    VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
        uint32_t index = *imageIndex;

        if (imagesInFlight[index] != VK_NULL_HANDLE) {
            vkWaitForFences(device().device(), 1, &imagesInFlight[index], VK_TRUE, UINT64_MAX);
        }

        imagesInFlight[index] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        vkResetFences(device().device(), 1, OUT &inFlightFences[currentFrame]);

        NVKG_ASSERT(vkQueueSubmit(device().graphics_queue(), 1, &submitInfo, OUT inFlightFences[currentFrame]) == VK_SUCCESS,
            "Failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] {swapChain};

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        auto result = vkQueuePresentKHR(device().present_queue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(VkSurfaceFormatKHR* formats, size_t formatCount) {
        for (size_t i = 0; i < formatCount; i++) {
            VkSurfaceFormatKHR& availableFormat = formats[i];
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return availableFormat;
        }
        return formats[0];
    }

    VkPresentModeKHR SwapChain::ChoosePresentMode(VkPresentModeKHR* presentModes, size_t presentModeCount) {
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

    VkExtent2D SwapChain::ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
        else {
            VkExtent2D actualExtent = windowExtent;
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

    VkFormat SwapChain::FindDepthFormat() {
        VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
        return device().find_supported_format(
            formats,
            3,
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
