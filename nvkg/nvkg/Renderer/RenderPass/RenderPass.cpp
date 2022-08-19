#include <nvkg/Renderer/RenderPass/RenderPass.hpp>

namespace nvkg {

    RenderPass::~RenderPass() {
        DestroyRenderPass();
    }

    void RenderPass::DestroyRenderPass() {
        vkDestroyRenderPass(device->device(), renderPass, nullptr);
    }

    void
    RenderPass::Begin(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, VkOffset2D offset, VkExtent2D extent,
                      VkClearValue *clearValues, uint32_t clearValueCount) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffer;

        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = extent;

        renderPassInfo.clearValueCount = clearValueCount;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(OUT commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::End(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(OUT commandBuffer);
    }

    RenderPass::RenderPass() = default;

    void RenderPass::Initialise(VulkanDevice *vulkanDevice, const RenderPass::Config& config) {
        device = vulkanDevice;

        auto& attachments = config.GetAttachments();
        auto& subPasses = config.GetSubPasses();
        auto& dependencies = config.GetDependencies();

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subPasses.size();
        renderPassCreateInfo.pSubpasses = subPasses.data();
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies = dependencies.data();

        NVKG_ASSERT(vkCreateRenderPass(device->device(), &renderPassCreateInfo, nullptr, OUT &renderPass) == VK_SUCCESS,
                    "Failed to create render pass!")
    }

    void RenderPass::Initialise(VulkanDevice* device, RenderPass &renderpass, const RenderPass::Config &config) {
        renderpass.Initialise(device, config);
    }

    // Config functions.

    RenderPass::Config &RenderPass::Config::WithAttachment(const VkAttachmentDescription& attachment) {
        attachments.push_back(attachment);
        return *this;
    }

    RenderPass::Config &RenderPass::Config::WithSubPass(const VkSubpassDescription& subpass) {
        subpasses.push_back(subpass);
        return *this;
    }

    RenderPass::Config &RenderPass::Config::WithDependency(const VkSubpassDependency& dependency) {
        dependencies.push_back(dependency);
        return *this;
    }
}