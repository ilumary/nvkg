#pragma once

#include <nvkg/Renderer/Core.hpp>

#include <vector>

namespace nvkg {
    
    namespace vertexdescription {

        inline VkVertexInputBindingDescription vertex_input_binding_description(
			uint32_t binding,
			uint32_t stride,
			VkVertexInputRate input_rate)
		{
			VkVertexInputBindingDescription v_input_bind_description {};
			v_input_bind_description.binding = binding;
			v_input_bind_description.stride = stride;
			v_input_bind_description.inputRate = input_rate;
			return v_input_bind_description;
		}

		inline VkVertexInputAttributeDescription vertex_input_attribute_description(
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset)
		{
			VkVertexInputAttributeDescription v_input_attrib_description {};
			v_input_attrib_description.location = location;
			v_input_attrib_description.binding = binding;
			v_input_attrib_description.format = format;
			v_input_attrib_description.offset = offset;
			return v_input_attrib_description;
		}

    } // namespace vertexdescription

    //TODO integrate in Pipeline class
    class PipelineConfig {
        public:

        struct ShaderConfig {
            VkShaderStageFlagBits stage;
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

        static VkPipelineShaderStageCreateInfo create_shader_stage(
            VkShaderStageFlagBits stage, 
            VkShaderModule module, 
            const char* pName = "main",
            VkPipelineShaderStageCreateFlags flags = 0,
            const void* pNext = nullptr,
            const VkSpecializationInfo* pSpecialisationInfo = nullptr
        );

        static void create_default_pipeline_stages(
            VkPipelineShaderStageCreateInfo* pShaderStageCreateInfos, 
            VkShaderStageFlagBits* stages,
            VkShaderModule* modules,
            uint32_t stageCount
        );

        private:
    };
}