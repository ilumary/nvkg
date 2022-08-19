#include <nvkg/Renderer/Pipeline/Pipeline.hpp>

#include <fstream>
#include <string>
#include <iostream>

namespace nvkg {
    Pipeline::Pipeline(
        const PipelineConfig::ShaderConfig* shaders,
        uint32_t shaderCount,
        const PipelineConfigInfo& configInfo
    ) : Pipeline() {
        CreateGraphicsPipeline(shaders, shaderCount, configInfo);
    }

    Pipeline::Pipeline() {}

    Pipeline::~Pipeline() {
        if (isFreed) return;

        ClearPipeline();

        isFreed = true;
    }

    void Pipeline::CreateGraphicsPipeline(
        const PipelineConfig::ShaderConfig* shaders,
        uint32_t shaderCount,
        const PipelineConfigInfo& configInfo) {

        NVKG_ASSERT(configInfo.pipelineLayout != VK_NULL_HANDLE, 
                "Cannot create graphics pipeline: no pipeline config provided in configInfo");
        
        NVKG_ASSERT(configInfo.renderPass != VK_NULL_HANDLE, 
                "Cannot create graphics pipeline: no renderpass config provided in configInfo");
        
        NVKG_ASSERT(shaderCount <= MAX_SHADER_MODULES, "Max allowed shader modules has been reached.");

        shaderModuleCount = shaderCount;
        
        VkPipelineShaderStageCreateInfo shaderStages[shaderCount];

        PipelineConfig::PipelineStage stages[shaderCount];

        for (size_t i = 0; i < shaderCount; i++) {
            if(shaders[i].filePath == nullptr) {
                shaderModules[i] = shaders[i].shader_module;
            } else {
                //auto shaderCode = ReadFile(shaders[i].filePath);
                //CreateShaderModule(shaderCode, OUT &shaderModules[i]);
            }

            stages[i] = shaders[i].stage;
        }

        PipelineConfig::CreateDefaultPipelineStages(OUT shaderStages, stages, shaderModules, shaderCount);

        // In order to pass in vertex information, we must assign a set of descriptions to the shader.
        // These descriptions detail all data binding and which locations these bindings must be set to. 
        // In our case, we have a generic set of descriptions for our Vertex structs. 

        // TODO: Maybe inject these into the config. Right now the pipeline is highly coupled 
        // to the models. 
        
        auto bindingDescriptions = configInfo.vertexData.bindings;
        auto attributeDescriptions = configInfo.vertexData.attributes;

        // Bind the descriptions to the pipeline to allow us to pass in vertex info via 
        // vertex buffers.
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        // Pass in all pipeline config details to the pipeline create info struct. 
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = shaderCount;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineCreateInfo.pViewportState = &configInfo.viewportInfo;
        pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineCreateInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineCreateInfo.pDynamicState = &configInfo.dynamicStateInfo;
        pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilInfo;

        pipelineCreateInfo.layout = configInfo.pipelineLayout;
        pipelineCreateInfo.renderPass = configInfo.renderPass;
        pipelineCreateInfo.subpass = configInfo.subPass;

        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        auto device = VulkanDevice::get_device_instance();

        NVKG_ASSERT(vkCreateGraphicsPipelines(device->device(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, OUT &graphicsPipeline) 
            == VK_SUCCESS, "Failed to create graphics pipeline!")
    }

    void Pipeline::RecreatePipeline(
        const PipelineConfig::ShaderConfig* shaders,
        uint32_t shaderCount,
        const PipelineConfigInfo& configInfo) {
        CreateGraphicsPipeline(shaders, shaderCount, configInfo);
    }

    void Pipeline::ClearPipeline() {
        auto device = VulkanDevice::get_device_instance();

        for (size_t i = 0; i < shaderModuleCount; i++) {
            vkDestroyShaderModule(device->device(), shaderModules[i], nullptr);
        }
        
        vkDestroyPipeline(device->device(), graphicsPipeline, nullptr);
    }

    void Pipeline::DestroyPipeline() {
        ClearPipeline();
        isFreed = true;
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    PipelineConfigInfo Pipeline::DefaultPipelineConfig() {
        PipelineConfigInfo configInfo {};

        configInfo.inputAssemblyInfo = PipelineConfig::InitInputAssemblyStateCreateInfo();
        
        configInfo.viewportInfo = 
                PipelineConfig::InitViewPortCreateInfo(1, nullptr, 1, nullptr);
        
        configInfo.rasterizationInfo = PipelineConfig::InitRasterizationCreateInfo(
            VK_FALSE, 
            VK_FALSE, 
            VK_POLYGON_MODE_FILL, 
            VK_CULL_MODE_NONE, 
            VK_FRONT_FACE_CLOCKWISE,
            VK_FALSE
        );

        configInfo.multisampleInfo = PipelineConfig::InitMultiSampleCreateInfo(
            VK_SAMPLE_COUNT_1_BIT, 
            VK_FALSE, 
            1.0f, 
            nullptr, 
            VK_FALSE, 
            VK_FALSE
        );

        // Configures that types of colors we handle. In our case we allow a 4-part vector containing
        // an R,G,B, and A value. 
        configInfo.colorBlendAttachment = PipelineConfig::InitColorBlendAttachment(
            VK_TRUE,
            VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            VK_BLEND_OP_ADD
        );

        // Specifies further configurations for color blending
        configInfo.colorBlendInfo = PipelineConfig::InitColorBlendCreateInfo(
            VK_FALSE, 
            VK_LOGIC_OP_COPY, 
            1, 
            &configInfo.colorBlendAttachment
        );

        // Determines how image depth is handled
        configInfo.depthStencilInfo = PipelineConfig::InitDepthStencilCreateInfo(
            VK_TRUE, 
            VK_TRUE, 
            VK_COMPARE_OP_LESS, 
            VK_FALSE, 
            VK_FALSE
        );

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        configInfo.dynamicStateInfo = PipelineConfig::InitDynamicStateCreateInfo(
            static_cast<uint32_t>(configInfo.dynamicStateEnables.size()),
            configInfo.dynamicStateEnables.data()
        );

        return configInfo;
    }
}