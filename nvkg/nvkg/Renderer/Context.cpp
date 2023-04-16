#include <nvkg/Renderer/Context.hpp>

namespace nvkg {

    std::vector<VkCommandBuffer> Context::command_buffers;

    Context::Context(Window& window, uint32_t thread_count) : window{window}, swapchain{SwapChain()} {
        device(&window);

        Input::init_with_window_pointer(&window);
        
        swapchain.set_window_extents(window.get_window_extent());

        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10);
        DescriptorPool::add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10);

        DescriptorPool::build_pool();

        renderer_ = std::make_unique<Renderer>();

        if(thread_count > 0)
            init_thread_data(thread_count);

        create_primary_cmdbf();
    }

    Context::~Context() {
        DescriptorPool::destroy_pool();

        MaterialManager::cleanup();
    }

    void Context::init_thread_data(uint32_t thread_count) {
        //create thread pool and leave (if available) a few threads to the system
        if(std::thread::hardware_concurrency() < 4) { 
            logger::debug(logger::Level::Warning) << "Detected less than 4 threads on the system. Performance may be impacted!";
            logger::debug(logger::Level::Warning) << "You may provide a custom thread count via the Context constructor";
        }
        uint32_t tc = (thread_count >= 1) ? thread_count : (std::thread::hardware_concurrency());
        thread_pool_ = std::make_unique<BS::thread_pool>(tc);
        logger::debug(logger::Level::Info) << "Creating thread pool with " << tc << " threads...";

        thread_data_.resize(thread_count); // TODO maybe not allocate all threads to rendering
        thread_command_buffer_collector.resize(thread_count); // one command buffer per thread data, therefore we can prevent per frame reallocation
        QueueFamilyIndices::QueueFamilyIndices queue_family_indices = device().find_phys_queue_families();

        VkCommandPoolCreateInfo command_pool_create_info = initializers::command_pool_create_info();
        command_pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family_;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        for(int i = 0; i < thread_count; i++) {
            //init command pool and secondary command buffer for every thread
            NVKG_ASSERT(
                vkCreateCommandPool(device().device(), &command_pool_create_info, nullptr, OUT &thread_data_[i].command_pool_) == VK_SUCCESS,
                "Failed to create command pool!"
            );

            VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = initializers::command_buffer_allocate_info(
                thread_data_[i].command_pool_, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);

            NVKG_ASSERT(vkAllocateCommandBuffers(device().device(), &secondary_cmd_buffer_alloc_info, &thread_data_[i].command_buffer_) == VK_SUCCESS,
                "Failed to allocate secondary command buffer");
        }
    }

    void Context::create_primary_cmdbf() {
        command_buffers = std::vector<VkCommandBuffer>(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info = initializers::command_buffer_allocate_info(
            device().get_command_pool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(command_buffers.size()));

        NVKG_ASSERT(vkAllocateCommandBuffers(device().device(), &alloc_info, OUT command_buffers.data()) == VK_SUCCESS,
            "Failed to allocate primary command buffer");
    }

    void Context::render_frame() {
        auto commandBuffer = get_crnt_cmdbf();

        VkCommandBufferInheritanceInfo inheritance_info = initializers::command_buffer_inheritance_info();
        inheritance_info.renderPass = swapchain.get_render_pass()->get();
        inheritance_info.framebuffer = swapchain.get_frame_buffer(current_image_index);

        /// Multithreaded render code

        VkCommandBufferBeginInfo cmd_bf_begin_info = initializers::command_buffer_begin_info();
        cmd_bf_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        cmd_bf_begin_info.pInheritanceInfo = &inheritance_info;

        /// End multithreaded cender code

        renderer_->render(commandBuffer, camera_, registry_);
    }

    void Context::recreate_swapchain() {
        clear_device_queue();
        auto extent = window.get_window_extent();
        while(extent.width == 0 || extent.height == 0) {
            extent = window.get_window_extent();
            window.await_events();
        }

        auto oldImageFormat = swapchain.get_image_format();
        auto oldDepthFormat = swapchain.get_depth_format();

        // Re-create swapchain
        swapchain.recreate_swapchain();

        // Re-create the pipeline once the swapchain renderpass 
        // becomes available again.
        if (!swapchain.compare_swap_formats(oldImageFormat, oldDepthFormat)) {
            renderer_->recreate_materials();
        }
    }

    void Context::free_cmdbf() {
        vkFreeCommandBuffers(
            device().device(), 
            device().get_command_pool(), 
            SwapChain::get_image_count(),
            command_buffers.data());
    }

    void Context::render() {
        new_time_ = std::chrono::high_resolution_clock::now();
        frame_time_ = std::chrono::duration<float, std::chrono::seconds::period>(new_time_ - current_time_).count();
        current_time_ = new_time_;

        if(!start_frame()) return;
                
        render_frame();

        end_frame();

        if(Input::mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
            auto pos = Input::get_cursor_pos();
            float deltaY = (old_cursor_pos.first - pos.first) * camera_->rotationSpeed * 0.5f;
            float deltaX = (old_cursor_pos.second - pos.second) * camera_->rotationSpeed * 0.5f;
            camera_->rotate(glm::vec3(deltaX, 0.0f, 0.0f));
            camera_->rotate(glm::vec3(0.0f, -deltaY, 0.0f));
        }
        
        camera_->update(frame_time_);

        old_cursor_pos = Input::get_cursor_pos();
    }

    bool Context::start_frame() {
        NVKG_ASSERT(!is_frame_started, "Can't start a frame when a frame is already in progress!");

        auto result = swapchain.acquire_next_image(&current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return false;
        }

        NVKG_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, 
            "Failed to acquire swapchain image!");

        is_frame_started = true;

        VkCommandBuffer commandBuffer = get_crnt_cmdbf();

        VkCommandBufferBeginInfo cmd_buffer_begin_info = initializers::command_buffer_begin_info();

        NVKG_ASSERT(vkBeginCommandBuffer(OUT commandBuffer, &cmd_buffer_begin_info) == VK_SUCCESS,
            "Failed to begin recording command buffer");
        
        begin_swapchain_renderpass(commandBuffer);
        
        return true;
    }

    void Context::end_frame() {
        NVKG_ASSERT(is_frame_started, "Can't end frame while frame is not in progress!");
        
        VkCommandBuffer commandBuffer = get_crnt_cmdbf();
        
        end_swapchain_renderpass(commandBuffer);

        NVKG_ASSERT(vkEndCommandBuffer(OUT commandBuffer) == VK_SUCCESS,
            "Failed to record command buffer!");

        auto result = swapchain.submit_command_buffers(&commandBuffer, &current_image_index);

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

        RenderPass::Begin(swapchain.get_render_pass()->get(),
                          OUT commandBuffer,
                          swapchain.get_frame_buffer(current_image_index),
                          {0,0},
                          swapchain.get_swapchain_extent(),
                          clear_values,
                          clear_value_count);

        VkViewport viewport = initializers::viewport(swapchain.get_width(), swapchain.get_height(), 0.0f, 1.0f);
        VkRect2D scissor = initializers::rect2D(swapchain.get_width(), swapchain.get_height(), 0, 0);

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Context::end_swapchain_renderpass(VkCommandBuffer commandBuffer) {
        NVKG_ASSERT(is_frame_started, "Can't end render pass while the frame hasn't started!");
        NVKG_ASSERT(commandBuffer == get_crnt_cmdbf(), "Can't begin a render pass on a command buffer from another frame!");
        
        RenderPass::End(commandBuffer);
    }
}
