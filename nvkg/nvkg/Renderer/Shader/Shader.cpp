#include <nvkg/Renderer/Shader/Shader.hpp>

namespace nvkg {

    std::vector<uint32_t> convert(std::vector<char> buf) {
        std::vector<uint32_t> output(buf.size() / sizeof(uint32_t));
        std::memcpy(output.data(), buf.data(), buf.size());
        return output;
    }

	void ShaderModule::create(std::string name, std::string stage, std::string entrance_function) {
		filename = name;
        this->entrance_function = entrance_function;
        auto device_ = VulkanDevice::get_device_instance();
        device = device_;

        if (stage == "vert")
            shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
        else if (stage == "frag")
            shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        else if (stage == "comp")
            shader_stage = VK_SHADER_STAGE_COMPUTE_BIT;
        else if (stage == "tess")
            shader_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; // todo: theres also tess_evaluation_bit
        else if (stage == "geom")
            shader_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        else if (stage == "glsl")
            shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS; // todo: dont know what to put here

        //TODO: extract to config
        std::string dir = "shaders/";

		filepath = dir + name + "." + stage + ".spv";

        binary_data = loadSpirVBinary(filepath);

        if (stage == "frag")
            reflect_descriptor_types(convert(binary_data), VK_SHADER_STAGE_FRAGMENT_BIT);

        if (stage == "vert")
            reflect_descriptor_types(convert(binary_data), VK_SHADER_STAGE_VERTEX_BIT);

        std::cout << "SHADER INFO: " << name << "." << stage << std::endl;
        std::cout << "UBO's: " << uniforms.size() << " with overall size " << combined_uniform_size << std::endl;
        std::cout << "Vertex Bindings: " << vertex_bindings.size() << std::endl;
        std::cout << "Push Constants: " << push_constants.size() << std::endl;

		VkShaderModuleCreateInfo shader_module_create_info = {};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = binary_data.size();
		shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(binary_data.data());

		NVKG_ASSERT(vkCreateShaderModule(device->device(), &shader_module_create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to create shader module");
	}

	void ShaderModule::cleanup() {
		if (shader_module != VK_NULL_HANDLE)
            vkDestroyShaderModule(device->device(), shader_module, nullptr);
	}

	///////////////////////////////////////////////////////////////////////////////////////////// Private

	std::vector<char> ShaderModule::loadSpirVBinary(std::string file_name) {
		std::ifstream file(file_name, std::ios::ate | std::ios::binary);
		NVKG_ASSERT(file.is_open(), "Vulkan Error: failed to open Spir-V file: " + file_name);

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		NVKG_ASSERT(!buffer.empty(), "Vulkan Error: Spir-V file empty: " + file_name);
		file.close();
		return buffer;
	}

    //TODO expand functionality
    void ShaderModule::reflect_descriptor_types(std::vector<uint32_t> spirv_binary, VkShaderStageFlagBits shader_stage) {
        spirv_cross::CompilerGLSL glsl(spirv_binary);
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        size_t min_device_alignment = VulkanDevice::get_device_instance()->get_device_alignment();

        // Get all push constants
        for (auto &resource : resources.push_constant_buffers) {
            auto ranges = glsl.get_active_buffer_ranges(resource.id);
            std::string name = glsl.get_name(resource.id);

            VkPushConstantRange push_constant_range = {};

            const spirv_cross::SPIRType &type = glsl.get_type(resource.base_type_id);
            if(type.basetype == spirv_cross::SPIRType::Struct) {
                size_t push_size = glsl.get_declared_struct_size(type);

                std::cout << "Push Constant struct " << name << " with size " << push_size << std::endl;

                push_constant_range.offset = 0;
                push_constant_range.size = push_size;
                push_constant_range.stageFlags = shader_stage;

                push_constants.push_back(push_constant_range);
            } else {
                for (auto &r : ranges) {
                    push_constant_range.offset = r.offset;
                    push_constant_range.size = r.range;
                    push_constant_range.stageFlags = shader_stage;

                    push_constants.push_back(push_constant_range);
                }
            }
        }
        
        // Get all sampled uniform buffers
        for (auto &resource : resources.uniform_buffers) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);
            Utils::Descriptor::DescriptorInfo descriptor_info = { binding, name, shader_stage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER };

            const spirv_cross::SPIRType &type = glsl.get_type(resource.base_type_id);
            size_t ubo_size = glsl.get_declared_struct_size(type);

            material_descriptor_orderings.push_back(descriptor_info);

            unsigned member_count = type.member_types.size();
            for (unsigned i = 0; i < member_count; i++) {
                auto &member_type = glsl.get_type(type.member_types[i]);
                size_t member_size = glsl.get_declared_struct_member_size(type, i);

                //account for min device alignment padding with non-array types
                if (member_type.array.empty()) {
                    if(member_size % min_device_alignment != 0) {
                        ubo_size += (min_device_alignment - member_size);
                    }
                }

                const std::string &name = glsl.get_member_name(type.self, i);

                std::cout << "Struct member " << name << " with size " << member_size << std::endl;
            }

            Utils::StringId strId = INTERN_STR(name.c_str());
            auto padded_size = Buffer::pad_uniform_buffer_size(ubo_size);
            uniforms.push_back(Uniform{strId, binding, padded_size, 1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
            combined_uniform_size += padded_size;

            std::cout << "UBO: set " << set << ", binding " << binding << ", name " << name << " and size " << ubo_size << std::endl;
        }

        // Get all sampled images
        for (auto &resource : resources.sampled_images) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);
            Utils::Descriptor::DescriptorInfo descriptor_info = { binding, name, shader_stage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

            std::cout << "Sample Image: set " << set << ", binding " << binding << ", name " << name << std::endl;

            material_descriptor_orderings.push_back(descriptor_info);
        }

        // Get all storage buffers
        for(auto& resource : resources.storage_buffers) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);
            Utils::Descriptor::DescriptorInfo descriptor_info = { binding, name, shader_stage, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC };

            material_descriptor_orderings.push_back(descriptor_info);

            const spirv_cross::SPIRType &type = glsl.get_type(resource.base_type_id);
            size_t ubo_size = glsl.get_declared_struct_size(type);

            unsigned member_count = type.member_types.size();
            for (unsigned i = 0; i < member_count; i++) {
                auto &member_type = glsl.get_type(type.member_types[i]);
                size_t member_size = glsl.get_declared_struct_member_size(type, i);

                //account for min device alignment padding with non-array types
                if (member_type.array.empty()) {
                    if(member_size % min_device_alignment != 0) {
                        ubo_size += (min_device_alignment - member_size);
                    }
                }

                const std::string &name = glsl.get_member_name(type.self, i);

                std::cout << "Struct member " << name << " with size " << member_size << std::endl;
            }

            Utils::StringId strId = INTERN_STR(name.c_str());
            auto padded_size = Buffer::pad_uniform_buffer_size(ubo_size);
            uniforms.push_back(Uniform{strId, binding, padded_size, 1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
            combined_uniform_size += padded_size;

            std::cout << "Storage Buffer: set " << set << ", binding " << binding << ", name " << name << ", size " << ubo_size << std::endl;
        }

        if(shader_stage == VK_SHADER_STAGE_VERTEX_BIT) {
            uint32_t inputs_size = 0, locations = 0;
            VertexBinding n_binding{};

            // Get all stage inputs
            for(auto& resource : resources.stage_inputs) {
                locations += 1;
                unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                unsigned location = glsl.get_decoration(resource.id, spv::DecorationLocation);
                std::string name = glsl.get_name(resource.id);

                const spirv_cross::SPIRType &type = glsl.get_type(resource.type_id);

                auto attrib_type = VertexDescription::AttributeType::VEC2;
                if(type.vecsize == 3) {
                    attrib_type = VertexDescription::AttributeType::VEC3;
                } else if(type.vecsize == 4) {
                    attrib_type = VertexDescription::AttributeType::VEC4;
                }

                auto attrib = VertexDescription::Attribute {
                    .offset = inputs_size,
                    .type = attrib_type,
                };

                n_binding.attributes.push_back(attrib);

                inputs_size += type.vecsize * 4; // because float = 4B

                std::cout << "Stage Input: set " << set << ", binding " << binding << ", name " << name << ", location " << location << ", vecsize " << type.vecsize << std::endl;
            }
            
            if(inputs_size > 0) {
                n_binding.vertexStride = inputs_size;
                vertex_bindings.push_back(n_binding);

                std::cout << "Added one vertex binding with " << locations << " locations and overall size of " << inputs_size << std::endl;
            }
                
        }

        // sort uniforms found by binding
        std::sort(uniforms.begin(), uniforms.end(),
            [](Uniform &l, Uniform &r) {
                return l.binding < r.binding;
            }
        );
    }
}