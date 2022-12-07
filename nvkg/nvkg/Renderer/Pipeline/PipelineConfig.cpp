#include <nvkg/Renderer/Pipeline/PipelineConfig.hpp>

namespace nvkg {
    VkVertexInputBindingDescription VertexDescription::CreateBinding(
            uint32_t binding,
            uint32_t stride, 
            VkVertexInputRate inputRate) {
        return { binding, stride, inputRate };
    }

    VkVertexInputAttributeDescription VertexDescription::CreateAttribute(
            uint32_t location, 
            uint32_t binding, 
            VkFormat format, 
            uint32_t offset) {
        return { location, binding, format, offset };
    }

    VertexDescription::Data VertexDescription::CreateDescriptions(size_t bindingCount, Binding* bindings) {
        Data vertexData;

        size_t totalAttributes = 0;
        for (size_t i = 0; i < bindingCount; i++) {
            totalAttributes += bindings[i].attributeCount;
        }

        vertexData.bindings = std::vector<VkVertexInputBindingDescription>(bindingCount);
        vertexData.attributes = std::vector<VkVertexInputAttributeDescription>(totalAttributes);

        size_t processedAttributes = 0;
        for (size_t i = 0; i < bindingCount; i++) {
            auto& binding = bindings[i];
            vertexData.bindings[i] = CreateBinding(i, binding.stride, (VkVertexInputRate)binding.inputRate);

            for (uint32_t j = 0; j < binding.attributeCount; j++) {
                auto attribute = binding.attributes[j];
                size_t attributeIndex = j + processedAttributes;
                vertexData.attributes[attributeIndex] = 
                    CreateAttribute(j, i, (VkFormat)attribute.type, attribute.offset);
            }

            processedAttributes += binding.attributeCount;
        }

        return vertexData;
    }

    VertexDescription::Binding VertexDescription::CreateBinding(
        uint32_t binding, 
        uint32_t stride, 
        InputRate inputRate, 
        Attribute* attributes, 
        uint32_t attibuteCount) {
        return { binding, stride, inputRate, attibuteCount, attributes};
    }

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