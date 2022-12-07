#include <nvkg/Renderer/Pipeline/Pipeline.hpp>

#include <fstream>
#include <string>
#include <iostream>

namespace nvkg {

    Pipeline::Pipeline() {}

    Pipeline::~Pipeline() {
        if (freed) return;
        clear();
        freed = true;
    }

    void Pipeline::create_graphics_pipeline( const PipelineConfig::ShaderConfig* shaders, uint32_t shader_count,
                const PipelineInit& p_config ) {

        NVKG_ASSERT(p_config.pipeline_layout != VK_NULL_HANDLE, 
                "Cannot create graphics pipeline: no pipeline config provided in configInfo");
        
        NVKG_ASSERT(p_config.render_pass != VK_NULL_HANDLE, 
                "Cannot create graphics pipeline: no renderpass config provided in configInfo");
        
        NVKG_ASSERT(shader_count <= MAX_SHADER_MODULES, "Max allowed shader modules has been reached.");

        shader_module_count = shader_count;

        VkPipelineShaderStageCreateInfo shader_stages[shader_count];
        VkShaderStageFlagBits stages[shader_count];

        for (size_t i = 0; i < shader_count; i++) {
            shader_modules[i] = shaders[i].shader_module;
            stages[i] = shaders[i].stage;
        }

        PipelineConfig::create_default_pipeline_stages(OUT shader_stages, stages, shader_modules, shader_count);

        auto binding_descriptions = p_config.vertex_data.bindings;
        auto attribute_descriptions = p_config.vertex_data.attributes;

        VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
        vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();
        vertex_input_create_info.pVertexBindingDescriptions = binding_descriptions.data();

        VkGraphicsPipelineCreateInfo pipelineCI = pipeline::pipeline_create_info(p_config.pipeline_layout, p_config.render_pass, 0);

        pipelineCI.pInputAssemblyState = &p_config.input_assembly_state;
		pipelineCI.pRasterizationState = &p_config.rasterization_state;
		pipelineCI.pColorBlendState = &p_config.color_blend_state;
		pipelineCI.pMultisampleState = &p_config.multisample_state;
		pipelineCI.pViewportState = &p_config.viewport_state;
		pipelineCI.pDepthStencilState = &p_config.depth_stencil_state;
		pipelineCI.pDynamicState = &p_config.dynamic_state;
        pipelineCI.pVertexInputState = &vertex_input_create_info;
        pipelineCI.stageCount = shader_count;
        pipelineCI.pStages = shader_stages;

        pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCI.basePipelineIndex = -1;

        NVKG_ASSERT(vkCreateGraphicsPipelines(VulkanDevice::get_device_instance()->device(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, OUT &pipeline) 
            == VK_SUCCESS, "Failed to create graphics pipeline!")
    }

    void Pipeline::clear() {
        auto device = VulkanDevice::get_device_instance();

        for (size_t i = 0; i < shader_module_count; i++) {
            vkDestroyShaderModule(device->device(), shader_modules[i], nullptr);
        }
        
        vkDestroyPipeline(device->device(), pipeline, nullptr);
    }

    void Pipeline::destroy() {
        clear();
        freed = true;
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    PipelineInit Pipeline::default_pipeline_init() {
        PipelineInit pc{};

        pc.input_assembly_state = pipeline::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        pc.rasterization_state = pipeline::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
        pc.blend_attachment_state = pipeline::pipeline_color_blend_attachment_state(0xf, VK_FALSE);
		pc.color_blend_state = pipeline::pipeline_color_blend_state_create_info(1, &pc.blend_attachment_state);
        pc.depth_stencil_state = pipeline::pipeline_depth_stencil_state_create_info(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        pc.viewport_state = pipeline::pipeline_viewport_state_create_info(1, 1, 0);
        pc.multisample_state = pipeline::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
        pc.dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        pc.dynamic_state = pipeline::pipeline_dynamic_state_create_info(pc.dynamic_state_enables);

        return pc;
    }

}