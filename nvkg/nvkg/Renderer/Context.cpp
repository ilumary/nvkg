#include <nvkg/Renderer/Context.hpp>

namespace nvkg {

    std::vector<VkCommandBuffer> Context::command_buffers;

    Context::Context(Window& window) : window{window}, swapchain{SwapChain(device_)} {
        device_.set_window_ptr(&window);
        swapchain.SetWindowExtents(window.get_window_extent());

        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10);

        DescriptorPool::build_pool();

        renderer = new Renderer();

        create_cmdbf();
    }

    Context::~Context() {
        DescriptorPool::destroy_pool();
        renderer->destroy();
    }

    void Context::create_cmdbf() {
        command_buffers = std::vector<VkCommandBuffer>(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandPool = device_.get_command_pool(),
            .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
        };
        

        NVKG_ASSERT(vkAllocateCommandBuffers(device_.device(), &allocInfo, OUT command_buffers.data()) == VK_SUCCESS,
            "Failed to allocate command buffer");
    }

    void Context::render_frame() {
        auto commandBuffer = get_crnt_cmdbf();

        renderer->render(commandBuffer, active_scene);
    }

    void Context::recreate_swapchain() {
        clear_device_queue();
        auto extent = window.get_window_extent();
        while(extent.width == 0 || extent.height == 0) {
            extent = window.get_window_extent();
            window.await_events();
        }

        auto oldImageFormat = swapchain.GetImageFormat();
        auto oldDepthFormat = swapchain.GetDepthFormat();

        // Re-create swapchain
        swapchain.RecreateSwapchain();

        // Re-create the pipeline once the swapchain renderpass 
        // becomes available again.
        if (!swapchain.CompareSwapFormats(oldImageFormat, oldDepthFormat)) {
            renderer->recreate_materials();
        }
    }

    void Context::free_cmdbf() {
        vkFreeCommandBuffers(
            device_.device(), 
            device_.get_command_pool(), 
            SwapChain::GetImageCount(),
            command_buffers.data());
    }

    bool Context::start_frame() {
        NVKG_ASSERT(!is_frame_started, "Can't start a frame when a frame is already in progress!");

        auto result = swapchain.AcquireNextImage(&current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return false;
        }

        NVKG_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, 
            "Failed to acquire swapchain image!");

        is_frame_started = true;

        VkCommandBuffer commandBuffer = get_crnt_cmdbf();

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        NVKG_ASSERT(vkBeginCommandBuffer(OUT commandBuffer, &beginInfo) == VK_SUCCESS,
            "Failed to begin recording command buffer");
        
        begin_swapchain_renderpass(commandBuffer);
        
        return true;
    }

    void Context::end_frame() {
        NVKG_ASSERT(is_frame_started, "Can't end frame while frame is not in progress!");

        render_frame();

        VkCommandBuffer commandBuffer = get_crnt_cmdbf();

        end_swapchain_renderpass(commandBuffer);

        NVKG_ASSERT(vkEndCommandBuffer(OUT commandBuffer) == VK_SUCCESS,
            "Failed to record command buffer!");

        auto result = swapchain.SubmitCommandBuffers(&commandBuffer, &current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.resized()) {
            window.reset_resize();
            recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            NVKG_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer for drawing!");
        }

        is_frame_started = false;
        current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT; 
    }

    void Context::begin_swapchain_renderpass(VkCommandBuffer commandBuffer) {

        NVKG_ASSERT(is_frame_started, "Can't start render pass while the frame hasn't started!");
        NVKG_ASSERT(commandBuffer == get_crnt_cmdbf(), "Can't begin a render pass on a command buffer from another frame!");

        uint32_t clear_value_count = 2;
        VkClearValue clear_values[clear_value_count];
        clear_values[0].color = clearValue;
        clear_values[1].depthStencil = {1.0f, 0};

        RenderPass::Begin(swapchain.GetRenderPass()->GetRenderPass(),
                          OUT commandBuffer,
                          swapchain.GetFrameBuffer(current_image_index),
                          {0,0},
                          swapchain.GetSwapChainExtent(),
                          clear_values,
                          clear_value_count);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain.GetSwapChainExtent().width);
        viewport.height = static_cast<float>(swapchain.GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor {{0,0}, swapchain.GetSwapChainExtent()};

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Context::end_swapchain_renderpass(VkCommandBuffer commandBuffer) {
        NVKG_ASSERT(is_frame_started, "Can't end render pass while the frame hasn't started!");
        NVKG_ASSERT(commandBuffer == get_crnt_cmdbf(), "Can't begin a render pass on a command buffer from another frame!");
        
        RenderPass::End(commandBuffer);
    }

    Scene* Context::create_scene(std::string name, bool set_active) {
        scenes[name] = new Scene(name);
        if(set_active) { active_scene = scenes[name]; }
        return scenes[name];
    }
    
    Scene* Context::get_scene(std::string name) {
        return scenes[name];
    }
}
