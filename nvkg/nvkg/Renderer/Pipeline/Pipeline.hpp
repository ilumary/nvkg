#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Device/VulkanDevice.hpp>
#include <nvkg/Renderer/Pipeline/PipelineConfig.hpp>

namespace nvkg {
    /**
     * A configuration struct containing all relevant pipeline config data. 
     * The intention is for the struct to be pre-configured to match the user's
     * intent and then be passed into the graphics pipeline when being constructed. 
     * 
     * Configuration includes:
     * 1) viewport configuration
     * 2) viewport scissor configurations
     * 3) input assembly - how vertices are submitted to the shader
     * 4) resterization - how vertices are represented 
     * 5) multisampling - how we handle multi-sampling to avoid aliasing.
     * 6) color blending - how colors are blended together in the shader. 
     * 7) depth stencil - how depth is represented within the shader.
     * 8) A pipeline layout representing how data is laid out in the shader. 
     * 9) Our render pass.
     * 9) Any subpasses used by the pipeline. 
     **/
    struct PipelineConfigInfo {
        VkPipelineViewportStateCreateInfo viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

        //Utils::Array<VkDynamicState> dynamicStateEnables;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        // Dependent structs on other data. 

        VkPipelineLayout pipelineLayout{nullptr}; // needs descriptor sets & push constants
        VkRenderPass renderPass{nullptr};
        uint32_t subPass{0}; 

        VertexDescription::Data vertexData;
    };

    class Pipeline {
        public:

            Pipeline(
                const char* vertFilePath, 
                const char* fragFilePath, 
                const PipelineConfigInfo& configInfo
            );

            Pipeline(
                const PipelineConfig::ShaderConfig* shaders,
                uint32_t shaderCount,
                const PipelineConfigInfo& configInfo
            );

            Pipeline();
            ~Pipeline();

            Pipeline(const Pipeline&) = delete;
            Pipeline& operator=(const Pipeline&) = delete;

            /**
             * Creates a default pipelineConfig struct. This struct currently configures
             * a pipeline to draw a 2D shape with custom vertices (using a vertex buffer).
             * @param width the image width (generally accessible from the swapchain)
             * @param height the image height (generally accessible from the swapchain)
             * @returns a PipleineConfigInfo struct with a default configuration.
             **/
            static PipelineConfigInfo DefaultPipelineConfig();

            /**
             * Binds a pipeline to a command buffer. This tells out command buffers that 
             * all bound buffers after this should be submitted to the same pipeline. This 
             * in turn means that all data passed in is then used to draw objects related to
             * said pipeline. 
             * @param commandBuffer the command buffer being bound to.
             **/
            void bind(VkCommandBuffer commandBuffer);

            void RecreatePipeline(
                const char* vertFilePath, 
                const char* fragFilePath, 
                const PipelineConfigInfo& configInfo
            );

            void RecreatePipeline(
                const PipelineConfig::ShaderConfig* shaders,
                uint32_t shaderCount,
                const PipelineConfigInfo& configInfo
            );

            void ClearPipeline();
            void DestroyPipeline();

        private:

            static constexpr size_t MAX_SHADER_MODULES = 2;

            void CreateGraphicsPipeline(
                const PipelineConfig::ShaderConfig* shaders,
                uint32_t shaderCount,
                const PipelineConfigInfo& configInfo
            );

            VkPipeline graphicsPipeline;

            VkShaderModule shaderModules[MAX_SHADER_MODULES];
            size_t shaderModuleCount = 0; 

            bool isFreed = false;
    };
}