#pragma once

#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/RenderPass/RenderPass.hpp>
#include <nvkg/Renderer/Image/FrameImages.hpp>

namespace nvkg {

    namespace initializers {

        inline VkViewport viewport(float width, float height, float minDepth, float maxDepth) {
			VkViewport viewport {};
			viewport.width = width;
			viewport.height = height;
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}

        inline VkRect2D rect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY) {
			VkRect2D rect2D {};
			rect2D.extent.width = width;
			rect2D.extent.height = height;
			rect2D.offset.x = offsetX;
			rect2D.offset.y = offsetY;
			return rect2D;
		}

    }

    /**
     * @brief The Swapchain class defines the creation and usage of a graphics swapchain. 
     * Swapchains define how images are rendered to screen. Generally, rendered objects are
     * drawn to images, which are contained within command buffers. Each frame, an image is drawn
     * to a command buffer which is then presented for rendering.
     * 
     * When an image takes longer than the given time for a frame, it runs the risk of overlapping
     * with the image being written in the following frame. This often results in screen tearing, 
     * where two images are overlain on top of each other. 
     * 
     * The swapchain is therefore responsible for buffering these images by writing to a new commandbuffer 
     * each frame. The new commandbuffer is then swapped out each frame depending on the method used. This
     * results in smooth image transitions and safer memory usage, as frames no longer compete to render the same data. 
     * 
     * Swapchains are essential for computer graphics processing. 
     */
    class SwapChain {
        public:

        /** 
         *  When swapping out images for our frames, we can have multiple frames 'in flight', 
         *  meaning frames that act as additional memory buffers. In our case we allow for two additional
         *  frames in flight for added image buffering.
         **/
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        // 'Structors 

        /**
         * @brief Creates a new Swapchain object to buffer our images. 
         * 
         * @param device a reference to a device. Devices are always destroyed last, so we can 
         * guarantee that object destruction will occur at the correct time. 
         * @param windowExtent the window resolution. Images need to be scaled to an extent, this usually
         * must follow the extents of the window. 
         */
        SwapChain(VkExtent2D windowExtent);
        SwapChain();
        ~SwapChain();

        // Delete copy constructors. 
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain*) = delete;

        // Getters

        /**
         * @brief Images can be formatted in a number of ways. The renderer currently 
         * searches for a specific format to render to. This function returns the format
         * currently being used by the renderer.
         * 
         * @return VkFormat the format being used by the renderer. 
         */
        VkFormat get_swapchain_image_format() { return swapchain_images_.GetFormat(); }

        /**
         * @brief Returns the image width and height that images are being rendered to. 
         * 
         * @return VkExtent2D a 2D representation of the height and width of the screen. 
         */
        VkExtent2D get_swapchain_extent() { return swapchain_extent_; }

        float get_width() const { return static_cast<float>(swapchain_extent_.width); }

        float get_height() const { return static_cast<float>(swapchain_extent_.height); }
        
        /**
         * @brief Get a frame buffer object corresponding to index i. The swapchain object stores multiple
         * frame buffers equal to the number of images that can be held at once. 
         * 
         * @param i the index of the required framebuffer. NOTE: this can return a segfault if an incorrect index is provided. Please ensure that the index provided
         * is between 0 and the swapchain's max image count. 
         * @return VkFramebuffer the framebuffer in index i
         */
        VkFramebuffer get_frame_buffer(uint32_t i) { return swapchain_frame_buffers_[i]; }

        /**
         * @brief Get the number of images that can be active at once. 
         * 
         * @return u32 representing the number of possible active simultaneous images. 
         */
        static uint32_t get_image_count() { return FrameImages::GetImageCount(); }

        /**
         * @brief The the raw Vulkan swapchain struct. 
         * 
         * @return VkSwapchainKHR 
         */
        VkSwapchainKHR get_swapchain() { return swapchain_; }

        static SwapChain* get_instance() { return instance_; }

        /**
         * @brief Get the current render pass object. 
         * 
         * @return RenderPass - the raw vulkan render pass object.
         */
        RenderPass* get_render_pass() { return &renderpass; }

        void set_window_extents(VkExtent2D windowExtent);

        /**
         * @brief Loads in the next image to be written to in the Swapchain. 
         * 
         * @param imageIndex - the index of the next image. 
         * @return VkResult - the result of acquiring the next image. 
         */
        VkResult acquire_next_image(uint32_t* imageIndex);

        /**
         * @brief Submits a command buffer for drawing.
         * 
         * @param buffers - an array of command buffers. 
         * @param imageIndex - the index of the image being drawn. 
         * @return VkResult - the result of submitting the buffer. 
         */
        VkResult submit_command_buffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        void recreate_swapchain();

        bool compare_swap_formats(VkFormat oldImageFormat, VkFormat oldDepthFormat);

        VkFormat get_image_format() { return swapchain_image_format_; }
        VkFormat get_depth_format() { return swapchain_depth_format_; }

        float extent_aspect_ratio() const {
            return get_width() / get_height();
        }

        private:
        
        void init();

        /**
         * @brief Create a Vulkan swapchain object. 
         */
        void create_swapchain();

        /**
         * @brief Create image views for our images. An image view represents the subresources
         * that images must have to be valid. An image view must be created for each image being drawn.
         */
        void create_image_views();

        /**
         * @brief Creates a render pass. In Vulkan, render passes are objects that represent 
         * the state that rendering resources must be in to render to an image. All image rendering
         * must occur within a render pass, while compute operations do not. 
         */
        void create_renderpass();

        /**
         * @brief Create a Depth Resources object. Depth resources represent how object depth is calculated.
         * These resources are written to their own resource views. 
         */
        void create_depth_resources();

        /**
         * @brief Creates the swapchain's frame buffers. A frame buffer represents all the data needed to be submitted 
         * to the GPU in a frame. 
         */
        void create_frame_buffers();

        /**
         * @brief Initialises all thread synchronisation objects. Vulkan is multi-threaded and submits images
         * asynchronously. This can be dangerous when submitting images to the GPU since Vulkan does not guarantee
         * safe execution. Therefore we must handle our own thread synchronisation. 
         */
        void create_sync_objects();

        /**
         * @brief Specifies which color format we want images to be written to. Accepts a set of formats
         * and populates them with requisite data. 
         * 
         * @param formats - an array of possible formats. 
         * @param formatCount - the number of formats provided. 
         * @return VkSurfaceFormatKHR - the chosen 
         */
        VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR* formats, size_t formatCount);

        /**
         * @brief Chooses the presentation mode that the window surface will use for images. 
         * 
         * @param presentModes - an array of possible presentation modes. 
         * @param presentModeCount  - the number of presentation modes in the array. 
         * @return VkPresentModeKHR - the chosen presentation mode. 
         */
        VkPresentModeKHR choose_present_mode(VkPresentModeKHR* presentModes, size_t presentModeCount);

        /**
         * @brief Chooses an image size extent that's supported by the swapchain.
         * 
         * @param capabilities a reference to a struct containing our swapchain capabilities. 
         * @return VkExtent2D - an object containing the swapchain's supported extents. 
         */
        VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR& capabilities);

        /**
         * @brief Finds the depth format supported by this swapchain. 
         * 
         * @return VkFormat - the supported image format. 
         */
        VkFormat find_depth_format();

        void clear_swapchain(bool isRecreated = false);
        void clear_memory();

        VkExtent2D window_extent_;

        static SwapChain* instance_;

        // frame buffers and renderpasses
        // TODO: Refactor this into a HeapArray
        VkFramebuffer* swapchain_frame_buffers_ {VK_NULL_HANDLE};
        RenderPass renderpass;

        VkFormat swapchain_image_format_;
        VkFormat swapchain_depth_format_;

        VkExtent2D swapchain_extent_;

        // Images and image views supported by the swapchain
        FrameImages swapchain_images_;
        // Depth image data.
        FrameImages depth_images_;

        // The raw Vulkan swapchain object
        VkSwapchainKHR swapchain_ {VK_NULL_HANDLE};

        // Synchronisation objects
        VkSemaphore* image_available_semaphores_ {VK_NULL_HANDLE};
        VkSemaphore* render_finished_semaphores_ {VK_NULL_HANDLE};
        VkFence* in_flight_fences_ {VK_NULL_HANDLE};
        VkFence* images_in_flight_ {VK_NULL_HANDLE};
        size_t current_frame_ = 0;
    };
}
