#pragma once

#include <nvkg/Renderer/Core.hpp>

#include <vector>

namespace nvkg {
    // TEMPORARY
    class VertexDescription {
        public:

        enum AttributeType {
            VEC2 = VK_FORMAT_R32G32_SFLOAT,
            VEC3 = VK_FORMAT_R32G32B32_SFLOAT,
            VEC4 = VK_FORMAT_R32G32B32A32_SFLOAT
        };

        enum InputRate {
            VERTEX = VK_VERTEX_INPUT_RATE_VERTEX,
            INSTANCE = VK_VERTEX_INPUT_RATE_INSTANCE
        };

        struct Attribute {
            uint32_t offset = 0;
            AttributeType type;
        };

        struct Binding {
            uint32_t binding;
            uint32_t stride;
            InputRate inputRate;
            uint32_t attributeCount;
            Attribute* attributes; 
        };
        
        struct Data {
            std::vector<VkVertexInputBindingDescription> bindings{};
            std::vector<VkVertexInputAttributeDescription> attributes{};
        };

        static Data CreateDescriptions(size_t bindingCount, Binding* bindings);

        static Binding CreateBinding(
            uint32_t binding, 
            uint32_t stride, 
            InputRate inputRate, 
            Attribute* attributes,
            uint32_t attibuteCount);

        static VkVertexInputBindingDescription CreateBinding(
            uint32_t binding,
            uint32_t stride, 
            VkVertexInputRate inputRate
        );

        static VkVertexInputAttributeDescription CreateAttribute(
            uint32_t location, 
            uint32_t binding, 
            VkFormat format, 
            uint32_t offset
        );
    };   

    //TODO integrate in Pipeline class
    class PipelineConfig {
        public:
        enum PipelineStage {
            VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
            FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT
        };

        struct ShaderConfig {
            const char* filePath { nullptr }; 
            PipelineStage stage;
            VkShaderModule shader_module { VK_NULL_HANDLE };
        };

        static VkPipelineLayoutCreateInfo create_pipeline_layout_create_info(
            VkDescriptorSetLayout* layouts, 
            uint32_t layoutCount, 
            VkPushConstantRange* pushConstants, 
            uint32_t pushConstantCount
        );

        static void create_pipeline_layout(
            VkDevice device,
            VkPipelineLayout* pipelineLayout,
            VkDescriptorSetLayout* layouts = nullptr, 
            uint32_t layoutCount = 0, 
            VkPushConstantRange* pushConstants = nullptr, 
            uint32_t pushConstantCount = 0
        );

        static VkPipelineShaderStageCreateInfo CreateShaderStage(
            VkShaderStageFlagBits stage, 
            VkShaderModule module, 
            const char* pName = "main",
            VkPipelineShaderStageCreateFlags flags = 0,
            const void* pNext = nullptr,
            const VkSpecializationInfo* pSpecialisationInfo = nullptr
        );

        static void CreateDefaultPipelineStages(
            VkPipelineShaderStageCreateInfo* pShaderStageCreateInfos, 
            PipelineStage* stages, 
            VkShaderModule* modules,
            uint32_t stageCount
        );

        private:
    };
}