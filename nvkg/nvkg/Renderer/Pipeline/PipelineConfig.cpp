#include <nvkg/Renderer/Pipeline/PipelineConfig.hpp>

namespace nvkg {

    VkPipelineLayoutCreateInfo PipelineConfig::create_pipeline_layout_create_info(
            VkDescriptorSetLayout* layouts, 
            uint32_t layoutCount, 
            VkPushConstantRange* pushConstants, 
            uint32_t pushConstantCount) {
        auto sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        return { sType, nullptr, 0, layoutCount, layouts, pushConstantCount, pushConstants};
    }

    void PipelineConfig::create_pipeline_layout(
            VkDevice device,
            VkPipelineLayout* pipelineLayout,
            VkDescriptorSetLayout* layouts, 
            uint32_t layoutCount, 
            VkPushConstantRange* pushConstants, 
            uint32_t pushConstantCount
    ) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
                create_pipeline_layout_create_info(layouts, layoutCount, pushConstants, pushConstantCount);

        NVKG_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout) == VK_SUCCESS, 
            "Failed to create pipeline layout!");
    }

    VkPipelineShaderStageCreateInfo PipelineConfig::create_shader_stage(
            VkShaderStageFlagBits stage, 
            VkShaderModule module, 
            const char* pName,
            VkPipelineShaderStageCreateFlags flags,
            const void* pNext,
            const VkSpecializationInfo* pSpecialisationInfo
    ) {
        auto sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        return 
        {
            sType, 
            pNext, 
            flags, 
            stage, 
            module, 
            pName, 
            pSpecialisationInfo
        };
    }

    void PipelineConfig::create_default_pipeline_stages(
            VkPipelineShaderStageCreateInfo* pShaderStageCreateInfos,
            VkShaderStageFlagBits* stages,
            VkShaderModule* modules,
            uint32_t stageCount) {
        for (size_t i = 0; i < stageCount; i++) {
            pShaderStageCreateInfos[i] = create_shader_stage(stages[i], modules[i]);
        }
    }
}