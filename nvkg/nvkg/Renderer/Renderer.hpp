#pragma once

#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/Swapchain/Swapchain.hpp>
#include <nvkg/Renderer/Pipeline/Pipeline.hpp>
#include <nvkg/Renderer/Model/Model.hpp>
#include <nvkg/Renderer/Camera/Camera.hpp>
#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Lights/PointLight.hpp>
#include <nvkg/Renderer/Renderers/Renderer3D.hpp>
#include <nvkg/Renderer/Renderers/Renderer2D.hpp>
#include <nvkg/Renderer/DescriptorPool/DescriptorPool.hpp>

namespace nvkg {

    class Renderer {
        public:

            Renderer(Window& window);
            ~Renderer();

            void destroy_renderer();

            VulkanDevice& get_device() { return device_; }
            SwapChain& get_swapchain() { return swapchain; }

            int get_crnt_frame_index() const {
                NVKG_ASSERT(!is_frame_started, "Can't get frame index when frame is not in progress!")
                return current_frame_index;
            }

            float get_aspect_ratio() const { return swapchain.ExtentAspectRatio(); }

            void set_camera(Camera* camera) { main_camera = camera; }
            
            bool frame_started() { return is_frame_started; }

            VkCommandBuffer get_crnt_cmdbf() const { 
                NVKG_ASSERT(is_frame_started, "Can't get command buffer when frame is not in progress!");
                return commandBuffers[current_frame_index]; 
            }

            bool start_frane();
            void end_frame();

            void clear_device_queue() { vkDeviceWaitIdle(device_.device()); } 

            void SetClearValue(float r, float g, float b, float a) { clearValue = {r, g, b, a}; }

        private:
            static constexpr size_t MAX_OBJECT_TRANSFORMS = 1000;
            
            static VulkanDevice* deviceInstance;
            static std::vector<VkCommandBuffer> commandBuffers;
            
            VkClearColorValue clearValue {0, 0, 0, 1.f};

            void create_cmdbf();
            void record_cmdbf();
            void free_cmdbf();

            void recreate_swapchain();

            void begin_swapchain_renderpass(VkCommandBuffer commandBuffer);
            void end_swapchain_renderpass(VkCommandBuffer commandBuffer);

            void DrawFrame();

            nvkg::Window& window;
            
            VulkanDevice device_;
            SwapChain swapchain;

            uint32_t current_image_index;
            bool is_frame_started{false};
            int current_frame_index{0};

            Camera* main_camera;
    };
}
