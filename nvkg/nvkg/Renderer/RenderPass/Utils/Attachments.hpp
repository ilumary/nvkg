#pragma once

#include <nvkg/Renderer/Core.hpp>

#include <vector>

namespace nvkg {

    /**
     * @brief The Attachments class is intended to act as a static class full of utility functions for building Vulkan
     * objects. The class contains functions for:
     * 1) Creating subpasses.
     * 2) Creating subpass dependencies.
     * 3) Creating attachments.
     */
    class Attachments {
    public:

        /**
         * @brief A builder class for creating a VkSubpassDescription. This class follows the builder pattern and is
         * intended to utilise function chaining to create data members. For example:
         *
         * ```
         * VkSubpassDescription subpass = SubPassBuilder()
         *                                    .WithColorReference(...)
         *                                    .WithDepthReference(...)
         *                                    .BuildGraphicsSubPass()
         * ```
         */
        class SubPassBuilder {
        public:

            static constexpr uint32_t MAX_COLOR_REFERENCES = 2;

            /**
             * @brief A default constructor for the SubPassBuilder.
             */
            SubPassBuilder() = default;

            /**
             * @brief A default destructor for the SubPassBuilder.
             */
            ~SubPassBuilder() = default;

            /**
             * @brief Adds a color reference. A VkAttachmentReference is a struct in which the index of a needed
             * attachment is specified, along with the desired image layout when the attachment is complete. The maximum
             * number of color references is equal to the MAX_COLOR_REFERENCES static variable.
             * @param reference the VkAttachmentReference being added.
             * @return the current builder object.
             */
            SubPassBuilder& WithColorReference(VkAttachmentReference reference);

            /**
             * @brief Adds a depth reference. Only one depth reference can be used per subpass.
             * @param reference the VkAttachmentReference sttoring our depth data.
             * @return the current builder object.
             */
            SubPassBuilder& WithDepthReference(VkAttachmentReference reference);

            /**
             * @brief Builds a VkSubpassDescription using the inputted attachment references.
             * @return a VkSubpassDescription with all the inputted data.
             */
            VkSubpassDescription BuildGraphicsSubPass();
        private:
            // A subpass can contain multiple color references
            std::vector<VkAttachmentReference> colorReferences{};

            // A subpass can only contain a single depth reference.
            VkAttachmentReference depthReference {};
        };

        /**
         * @brief A builder class for creating a subpass dependency. Subpass dependencies allow us to specify an order
         * in which subpasses must be processed. If a subpass requires the output of another subpass, we can specify
         * which stages we need to wait for before starting the rendering process.
         *
         * This class follows the builder pattern and isvintended to utilise function chaining to create data members.
         * For example:
         *
         * ```
         * DependencyBuilder builder = DependencyBuilder()
         *                                 .WithSrcSubPass(...)
         *                                 .WithDstSubPass(...)
         *                                 ...
         *                                 .Build();
         * ```
         */
        class DependencyBuilder {
        public:
            /**
             * @brief A default constructor for the DependencyBuilder.
             */
            DependencyBuilder() = default;

            /**
             * @brief A default destructor for the DependencyBuilder.
             */
            ~DependencyBuilder() = default;

            /**
             * @brief Specifies the index of the subpass that we depend on. Execution of this subpass will not occur
             * until the subpass in this index is complete.
             * @param subpass the index of the subpass we depend on.
             * @return the current builder object.
             */
            DependencyBuilder& WithSrcSubPass(uint32_t subpass);

            /**
             * @brief Specifies the index of the current subpass. This will pick the appropriate subpass for us to use
             * once the dependant subpass is complete.
             * @param subpass the index of the current subpass.
             * @return the current builder object.
             */
            DependencyBuilder& WithDstSubPass(uint32_t subpass);

            /**
             * @brief Specifies which rendering processes need to be complete before moving onto the current subpass.
             * @param stageMask a set of stage flags which specify the stages which need to be complete before
             * evaulating this subpass.
             * @return the current builder object.
             */
            DependencyBuilder& WithSrcStageMask(VkPipelineStageFlags stageMask);

            /**
             * @brief Specifies which stage masks the current subpass will be executing. This creates a dependency where
             * the dstStageMasks can't be executed until the srcStageMasks are complete.
             * @param stageMask a set of stage flags which this subpass will execute.
             * @return the current builder object.
             */
            DependencyBuilder& WithDstStageMask(VkPipelineStageFlags stageMask);

            /**
             * @brief Specifies which memory access types will be used by the subpass we depend on.
             * @param accessMask a set of stage flags specifying the memory access type used.
             * @return the current builder object.
             */
            DependencyBuilder& WithSrcAccessMask(VkAccessFlags accessMask);

            /**
             * @brief Specifies the memory access types used by the current subpass after the subpass it depends on
             * is complete.
             * @param accessMask a set of stage flags specifying the memory access required.
             * @return the current builder object
             */
            DependencyBuilder& WithDstAccessMask(VkAccessFlags accessMask);

            /**
             * @brief Uses all data inserted into the builder and returns a VkSubpassDependency.
             * @return a VkSubpassDependency with all of the dependency data.
             */
            VkSubpassDependency Build();
        private:
            uint32_t srcSubpass;
            uint32_t dstSubpass;
            VkPipelineStageFlags srcStageMask;
            VkPipelineStageFlags dstStageMask;
            VkAccessFlags srcAccessMask;
            VkAccessFlags dstAccessMask;
        };

        /**
         * @brief A shorthand function for creating a SubPassBuilder.
         * @return an empty SubPassBuilder object.
         */
        static SubPassBuilder CreateSubPass() { return SubPassBuilder{}; }

        /**
         * @brief A shorthand function for creating a DependencyBuilder.
         * @return an empty DependencyBuilder object.
         */
        static DependencyBuilder CreateSubPassDependency() { return DependencyBuilder{}; }

        /**
         * @brief Creates a default Color Attachment.
         *
         * @param format The image format. This will be the standard image format that our swapchain supports
         * @return A VkAttachmentDescription with default color attachment settings
         */
        static VkAttachmentDescription CreateColorAttachment(VkFormat format);

        /**
         * @brief Creates a default Depth Attachment.
         *
         * @param format The image format. This will be a Depth Format
         * @return A VkAttachmentDescription with default depth attachment settings
         */
        static VkAttachmentDescription CreateDepthAttachment(VkFormat format);

        /**
         * @brief Creates an attachment with the provided attachment data. This is a utility function that creates a
         * VkAttachmentDescription.
         *
         * @param format The format we want to use for this attachment
         * @param samples The number of samples per pixel we want to use
         * @param loadOp Our load operation. This setting dictates what our expected starting state is when this operation occurs
         * @param storeOp Our Store operation. This dictates what we want to do with the results of this operation
         * @param stencilLoadOp If we have a stencil operation, this will dictate what our starting state is expeted to be
         * @param stencilStoreOp Dictates what we want to do with the end result of this stencil operation
         * @param initialLayout Specifies what layout we expect our image to be in when the operation starts
         * @param finalLayout Specifies the image layout we expect to see after the operation has concluded
         * @return
         */
        static VkAttachmentDescription CreateAttachment(VkFormat format,
                                                        VkSampleCountFlagBits samples,
                                                        VkAttachmentLoadOp loadOp,
                                                        VkAttachmentStoreOp storeOp,
                                                        VkAttachmentLoadOp stencilLoadOp,
                                                        VkAttachmentStoreOp stencilStoreOp,
                                                        VkImageLayout initialLayout,
                                                        VkImageLayout finalLayout);

        /**
         * @brief Creates an attachment reference.
         *
         * @param attachment The index of the attachment this reference is referring to
         * @param imageLayout The layout the attachment will be using during our subpass
         * @return A VkAttachmentReference struct
         */
        static VkAttachmentReference CreateAttachmentReference(uint32_t attachment, VkImageLayout imageLayout);

        /**
         * @brief Creates an attachment reference specifically configured for depth stencils.
         *
         * @param attachment the index of the attachment being referenced
         * @return a VkAttachmentReference struct configured for depth stencils
         */
        static VkAttachmentReference CreateDepthStencilAttachmentReference(uint32_t attachment);

        /**
         * @brief Creates an attachment reference specifically configured for rendering colors.
         *
         * @param attachment the index of the attachment being referenced
         * @return a VkAttachmentReference struct configured for colors
         */
        static VkAttachmentReference CreateColorAttachmentReference(uint32_t attachment);

        /**
         * @brief Creates a subpass for a binding with the configured attachments.
         *
         * @param bindPoint The type of instruction being referenced, for example a compute or graphics bind point
         * @param colorAttachmentCount The number of color attachments
         * @param colorAttachments An array of the color attachments being used
         * @param depthStencilAttachment The depth stencil attachment being used
         * @param resolveAttachments The resolve attachments being used
         * @param preserveAttachmentCount The amount of preserve attachments
         * @param preserveAttachments An array of our preserve attachments
         * @param inputAttachmentCount The amount of attachments being used
         * @param inputAttachments An array of the attachments being used
         * @param flags Any flags that have been configured
         *
         * @return A VkSubpassDescription description with our configured data
         */
        static VkSubpassDescription CreateSubpass(VkPipelineBindPoint bindPoint,
                                                  uint32_t colorAttachmentCount = 0,
                                                  const VkAttachmentReference* colorAttachments = VK_NULL_HANDLE,
                                                  const VkAttachmentReference* depthStencilAttachment = VK_NULL_HANDLE,
                                                  const VkAttachmentReference* resolveAttachments = VK_NULL_HANDLE,
                                                  uint32_t preserveAttachmentCount = 0,
                                                  const uint32_t* preserveAttachments = VK_NULL_HANDLE,
                                                  uint32_t inputAttachmentCount = 0,
                                                  const VkAttachmentReference* inputAttachments = VK_NULL_HANDLE,
                                                  VkSubpassDescriptionFlags flags = 0);

        /**
         * @brief Creates a subpass which binds to a graphics process
         *
         * @param colorAttachmentCount The number of color attachments
         * @param colorAttachments An array of the color attachments being used
         * @param depthStencilAttachment The depth stencil attachment being used
         * @param resolveAttachments The resolve attachments being used
         * @param preserveAttachmentCount The amount of preserve attachments
         * @param preserveAttachments An array of our preserve attachments
         * @param inputAttachmentCount The amount of attachments being used
         * @param inputAttachments An array of the attachments being used
         * @param flags Any flags that have been configured
         *
         * @return A VkSubpassDescription description configured for graphics processes
         */
        static VkSubpassDescription CreateGraphicsSubpass(uint32_t colorAttachmentCount = 0,
                                                  const VkAttachmentReference* colorAttachments = VK_NULL_HANDLE,
                                                  const VkAttachmentReference* depthStencilAttachment = VK_NULL_HANDLE,
                                                  const VkAttachmentReference* resolveAttachments = VK_NULL_HANDLE,
                                                  uint32_t preserveAttachmentCount = 0,
                                                  const uint32_t* preserveAttachments = VK_NULL_HANDLE,
                                                  uint32_t inputAttachmentCount = 0,
                                                  const VkAttachmentReference* inputAttachments = VK_NULL_HANDLE,
                                                  VkSubpassDescriptionFlags flags = 0);

        /**
         * @brief Creates a VkSubpassDependency object with the configured data
         *
         * @param srcSubpass The index of the first subpass
         * @param dstSubpass The index of the second subpass in the dependency
         * @param srcStageMask The graphics stage this subpass should start at
         * @param dstStageMask The graphics stage this subpass should end at
         * @param srcAccessMask The stage where memory should be accessed from initially
         * @param dstAccessMask The stage we should be in following the renderpass
         *
         * @return A VkSubpassDependency struct
         */
        static VkSubpassDependency CreateDependency(uint32_t srcSubpass,
                                                    uint32_t dstSubpass,
                                                    VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask,
                                                    VkAccessFlags srcAccessMask,
                                                    VkAccessFlags dstAccessMask);
    };
}

