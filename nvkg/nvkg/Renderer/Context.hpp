#ifndef NVKG_CONTEXT_HPP
#define NVKG_CONTEXT_HPP

#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/Swapchain/Swapchain.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Renderer/Renderer.hpp>
#include <nvkg/Renderer/DescriptorPool/DescriptorPool.hpp>
#include <nvkg/Input/Input.hpp>

#include <chrono>

namespace nvkg {

    class Context {
        public:

            Context(Window& window);
            ~Context();

            SwapChain& get_swapchain() { return swapchain; }

            int get_crnt_frame_index() const {
                NVKG_ASSERT(!is_frame_started, "Can't get frame index when frame is not in progress!")
                return current_frame_index;
            }

            float get_aspect_ratio() const { return swapchain.ExtentAspectRatio(); }

            ecs::registry& get_registry() { return registry_; }

            void render();
            
            bool frame_started() { return is_frame_started; }

            VkCommandBuffer get_crnt_cmdbf() const { 
                NVKG_ASSERT(is_frame_started, "Can't get command buffer when frame is not in progress!");
                return command_buffers[current_frame_index]; 
            }

            bool start_frame();
            void end_frame();

            void clear_device_queue() { vkDeviceWaitIdle(device().device()); } 

            void set_clear_value(float r, float g, float b, float a) { clearValue = {r, g, b, a}; }

            float get_frame_time() { return frame_time_; };

            void set_camera(std::shared_ptr<CameraNew> cam) { camera_ = cam; }

        private:
            static std::vector<VkCommandBuffer> command_buffers;
            
            VkClearColorValue clearValue {0, 0, 0, 1.f};

            void create_cmdbf();
            void record_cmdbf();
            void free_cmdbf();

            void recreate_swapchain();

            void begin_swapchain_renderpass(VkCommandBuffer commandBuffer);
            void end_swapchain_renderpass(VkCommandBuffer commandBuffer);

            void render_frame();

            nvkg::Window& window;
            
            SwapChain swapchain;

            std::unique_ptr<Renderer> renderer_;

            uint32_t current_image_index;
            bool is_frame_started{false};
            int current_frame_index{0};

            std::chrono::time_point<std::chrono::high_resolution_clock> new_time_, current_time_ = std::chrono::high_resolution_clock::now();
            float frame_time_ = 0.0f;
            std::pair<double, double> old_cursor_pos;

            ecs::registry registry_;
            std::shared_ptr<CameraNew> camera_;
    };
}

#endif
