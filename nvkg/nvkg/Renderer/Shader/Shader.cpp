#include <nvkg/Renderer/Shader/Shader.hpp>

namespace nvkg {

    bool glsl_runtime_compiler::preprocess_glsl(const shader_info& info, std::string& glsl_shader_code) {
        glslang::InitializeProcess();

        auto translate_stage = [](VkShaderStageFlagBits stage) -> EShLanguage {
            switch (stage) {
                case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return EShLanguage::EShLangVertex;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return EShLanguage::EShLangTessControl;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLanguage::EShLangTessEvaluation;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return EShLanguage::EShLangGeometry;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return EShLanguage::EShLangFragment;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return EShLanguage::EShLangCompute;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return EShLanguage::EShLangRayGen;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return EShLanguage::EShLangAnyHit;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return EShLanguage::EShLangClosestHit;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return EShLanguage::EShLangMiss;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return EShLanguage::EShLangIntersect;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return EShLanguage::EShLangCallable;
            default:
                NVKG_ASSERT(false, "Tried creating shader with unknown shader stage");
                return EShLanguage::EShLangCount;
            }
        };

        auto shader_stage = translate_stage(info.shader_stage_);
        auto shader = glslang::TShader { shader_stage };
        auto shader_code_c = info.shader_code_.c_str();

        shader.setStrings(&shader_code_c, 1);
        shader.setEnvInput(glslang::EShSource::EShSourceGlsl, shader_stage,
                glslang::EShClient::EShClientVulkan,
                glslang::EShTargetClientVersion::EShTargetVulkan_1_1);

        auto build_preamble = [](const std::vector<std::string>& defines) -> std::string {
            auto preamble = std::ostringstream{};
            
            for(const auto& define : defines) {
                preamble << "#define " << define << "\n";
            }

            return preamble.str();
        };

        const auto preamble = build_preamble(info.compilation_defines_);
        shader.setPreamble(preamble.c_str());

        shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_1);
        shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);
        shader.setEntryPoint(info.entry_.c_str());
        shader.setSourceEntryPoint("main");

        glslang::TShader::ForbidIncluder forbid_includer;

        shader.preprocess(&defaultTBuiltInResource_, 450, EProfile::ENoProfile, false, false,
                static_cast<EShMessages>(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules),
                &glsl_shader_code, forbid_includer);

        glslang::FinalizeProcess();

        return true;
    }

    bool glsl_runtime_compiler::compile_to_spirv(const shader_info& info, std::vector<uint32_t>& shader_code) {
        glslang::InitializeProcess();

        auto translate_stage = [](VkShaderStageFlagBits stage) -> EShLanguage {
            switch (stage) {
                case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return EShLanguage::EShLangVertex;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return EShLanguage::EShLangTessControl;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLanguage::EShLangTessEvaluation;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return EShLanguage::EShLangGeometry;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return EShLanguage::EShLangFragment;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return EShLanguage::EShLangCompute;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return EShLanguage::EShLangRayGen;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return EShLanguage::EShLangAnyHit;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return EShLanguage::EShLangClosestHit;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return EShLanguage::EShLangMiss;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return EShLanguage::EShLangIntersect;
                case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return EShLanguage::EShLangCallable;
            default:
                NVKG_ASSERT(false, "Tried creating shader with unknown shader stage");
                return EShLanguage::EShLangCount;
            }
        };

        auto shader_stage = translate_stage(info.shader_stage_);
        auto shader = glslang::TShader { shader_stage };
        auto shader_code_c = info.shader_code_.c_str();

        shader.setStrings(&shader_code_c, 1);
        shader.setEnvInput(glslang::EShSource::EShSourceGlsl, shader_stage,
                glslang::EShClient::EShClientVulkan,
                glslang::EShTargetClientVersion::EShTargetVulkan_1_1);

        auto build_preamble = [](const std::vector<std::string>& defines) -> std::string {
            auto preamble = std::ostringstream{};
            
            for(const auto& define : defines) {
                preamble << "#define " << define << "\n";
            }

            return preamble.str();
        };

        const auto preamble = build_preamble(info.compilation_defines_);
        shader.setPreamble(preamble.c_str());

        shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_1);
        shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);
        shader.setEntryPoint(info.entry_.c_str());
        shader.setSourceEntryPoint("main");

        const auto compilation_errors = !shader.parse(&defaultTBuiltInResource_, 450, false,
	        static_cast<EShMessages>(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules));

        if(compilation_errors) {
            logger::debug() << "Error: " << shader.getInfoLog();
            logger::debug() << "Error: " << shader.getInfoDebugLog();
            glslang::FinalizeProcess();
            return false;
        }

        auto spv_options = glslang::SpvOptions{};

        if(info.enable_debug_compilation_) {
            spv_options.generateDebugInfo = true;
            spv_options.disableOptimizer = true;
            spv_options.optimizeSize = false;
        } else {
            spv_options.generateDebugInfo = false;
            spv_options.disableOptimizer = false;
            spv_options.optimizeSize = true;
        }

        auto spirv_logger = spv::SpvBuildLogger{};
        glslang::GlslangToSpv(*shader.getIntermediate(), shader_code, &spirv_logger, &spv_options);

        const auto messages = spirv_logger.getAllMessages();
        if(!messages.empty()) {
            logger::debug() << messages;
        }

        glslang::FinalizeProcess();

        return true;
    }

    std::vector<uint32_t> convert(std::vector<char> buf) {
        std::vector<uint32_t> output(buf.size() / sizeof(uint32_t));
        std::memcpy(output.data(), buf.data(), buf.size());
        return output;
    }

    std::vector<char> convert(std::vector<uint32_t> buf) {
        std::vector<char> output(buf.size() * sizeof(uint32_t));
        std::memcpy(output.data(), buf.data(), buf.size());
        return output;
    }

    std::string read_file_to_string(const std::filesystem::path& path) {
        if(!std::filesystem::exists(path) && !std::filesystem::is_regular_file(path)) {
            logger::debug(logger::Level::Error) << path << " is an invalid path or invalid file";
            return {};
        }

        auto err = std::error_code{};
        auto filesize = std::filesystem::file_size(path, err);

        std::ifstream file;
        file.open(path);

        std::stringstream strStream;
        strStream << file.rdbuf();
        return strStream.str();
    }

    template <typename TP>
    std::time_t to_time_t(TP tp) {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
        return system_clock::to_time_t(sctp);
    }

    std::string file_time_to_string(std::filesystem::file_time_type& time) {
        std::time_t tt = to_time_t(time);
        std::tm *gmt = std::gmtime(&tt);
        std::stringstream buffer;
        buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M");
        return buffer.str();
    }

    ShaderModule::ShaderModule(std::string file, bool runtime_compilation) {
        if(runtime_compilation) {
            file_handle_.path_ = "../shaders/" + file;
        } else {
            file_handle_.path_ = "shaders/" + file + ".spv";
            file_handle_.compile_sources = false;
        }

        using namespace std::chrono_literals;

        file_handle_.last_write_time_ = std::filesystem::last_write_time(file_handle_.path_);
        std::filesystem::last_write_time(file_handle_.path_, file_handle_.last_write_time_ + 1h);

        std::string stage = file_handle_.path_.extension();
        stage.erase(std::remove(stage.begin(), stage.end(), '.'), stage.end());

        if(stage == "spv") {
            std::string tmp = file_handle_.path_;
            size_t spv_extension = tmp.find_last_of('.');
            tmp.erase(spv_extension, 4);
            std::filesystem::path new_path = tmp;
            stage = new_path.extension();
            stage.erase(std::remove(stage.begin(), stage.end(), '.'), stage.end());
        }

        if (stage == "vert") {
            shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
        } else if (stage == "frag") {
            shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        } else if (stage == "comp") {
            shader_stage = VK_SHADER_STAGE_COMPUTE_BIT;
        } else if (stage == "tess") {
            shader_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; // todo: theres also tess_evaluation_bit
        } else if (stage == "geom") {
            shader_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else if (stage == "glsl") {
            shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS; // todo: dont know what to put here
        } else {
            logger::debug(logger::Level::Error) << "No shader stage was found!";
        }

        recompile();
    }

    ShaderModule::~ShaderModule() {};

    void ShaderModule::recompile() {
        std::filesystem::file_time_type crnt_last_write_time = std::filesystem::last_write_time(file_handle_.path_);
        if(crnt_last_write_time > file_handle_.last_write_time_) {
            logger::debug() << "Upgrading outdated shader " << file_handle_.path_ << " from " << file_time_to_string(file_handle_.last_write_time_);
            if(file_handle_.compile_sources) {
                std::string shader_file = read_file_to_string(file_handle_.path_);
                logger::debug() << "Compiling shader...";
                if(glsl_runtime_compiler::compile_to_spirv({{}, shader_stage, shader_file, "main"}, spirv_bin_data_u32)) {
                    logger::debug() << "Shader compiled successfully!";
                    spirv_bin_data = convert(spirv_bin_data_u32);
                } else {
                    logger::debug() << "Error compiling shader!";
                    return; //TODO handle error correctly
                }
            } else {
                spirv_bin_data = loadSpirVBinary(file_handle_.path_);
                spirv_bin_data_u32 = convert(spirv_bin_data);
            }
            file_handle_.last_write_time_ = crnt_last_write_time;
            create();
        }
    }

	void ShaderModule::create() {
        if (shader_stage == VK_SHADER_STAGE_FRAGMENT_BIT)
            reflect_descriptor_types(spirv_bin_data_u32);

        if (shader_stage == VK_SHADER_STAGE_VERTEX_BIT)
            reflect_descriptor_types(spirv_bin_data_u32);

        logger::debug() << "SHADER INFO: " << file_handle_.path_;
        logger::debug() << "UBO's: " << shader_resources.size() << " with overall size " << combined_uniform_size;
        logger::debug() << "Vertex Attributes: " << vertex_attributes.attributes.size();
        logger::debug() << "Push Constants: " << push_constants_new.size();

		VkShaderModuleCreateInfo shader_module_create_info = {};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = spirv_bin_data.size();
		shader_module_create_info.pCode = spirv_bin_data_u32.data();

		NVKG_ASSERT(vkCreateShaderModule(device().device(), &shader_module_create_info, nullptr, &shader_module) == VK_SUCCESS, "Failed to create shader module");
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

    void ShaderModule::reflect_descriptor_types(std::vector<uint32_t>& spirv_binary) {
        spirv_cross::CompilerGLSL glsl(spirv_binary);
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        reflect_push_constants(glsl);
        reflect_uniform_buffers(glsl);
        reflect_storage_buffers(glsl);
        reflect_sampled_images(glsl);

        if(shader_stage == VK_SHADER_STAGE_VERTEX_BIT) {
            reflect_stage_inputs(glsl);
        }

        // sort uniforms by set and binding
        std::sort(shader_resources.begin(), shader_resources.end(),
            [](ShaderResource &l, ShaderResource &r) {
                if( l.set != r.set)
                    return (l.set < r.set);
                return (l.binding < r.binding);
            }
        );

        /*for(auto &resource : shader_resources) {
            std::cout << resource.set << " " << resource.binding << std::endl;
        }*/
    }

    void ShaderModule::reflect_stage_inputs(spirv_cross::CompilerGLSL &glsl) {
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        uint32_t inputs_size = 0, locations = 0;
        VertexAttributes n_attributes{};

        std::map<uint16_t, spirv_cross::Resource> sorted_stage_inputs;

        for(auto& resource : resources.stage_inputs) {
            unsigned location = glsl.get_decoration(resource.id, spv::DecorationLocation);
            sorted_stage_inputs[location] = resource;
        }

        for(auto& resource : sorted_stage_inputs) {
            locations += 1;
            unsigned set = glsl.get_decoration(resource.second.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.second.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.second.id);

            const spirv_cross::SPIRType &type = glsl.get_type(resource.second.type_id);
            
            auto match_attrib_type = [&type](uint32_t vecsize) -> VkFormat {
                if(vecsize == 1 && type.basetype == spirv_cross::SPIRType::BaseType::Float) return VK_FORMAT_R32_SFLOAT;
                if(vecsize == 2 && type.basetype == spirv_cross::SPIRType::BaseType::Float) return VK_FORMAT_R32G32_SFLOAT;
                if(vecsize == 3 && type.basetype == spirv_cross::SPIRType::BaseType::Float) return VK_FORMAT_R32G32B32_SFLOAT;
                if(vecsize == 4 && type.basetype == spirv_cross::SPIRType::BaseType::Float) return VK_FORMAT_R32G32B32A32_SFLOAT;
            };

            std::pair<uint32_t, VkFormat> attrib = { inputs_size, match_attrib_type(type.vecsize) };

            n_attributes.attributes.push_back(attrib);

            inputs_size += (type.width/8) * type.vecsize;
        }
        
        if(inputs_size > 0) {
            n_attributes.vertexStride = inputs_size;
            vertex_attributes = n_attributes;
        }
    }

    void ShaderModule::reflect_sampled_images(spirv_cross::CompilerGLSL &glsl) {
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();
        
        for (auto &resource : resources.sampled_images) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);

            Utils::StringId strId = INTERN_STR(name.c_str());
            shader_resources.push_back(ShaderResource{
                .id = strId, 
                .set = set,
                .binding = binding,
                .size =  0,
                .arraySize = 0,
                .dyn_count = 1,
                .offset = 0,
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .stage = (VkShaderStageFlags)shader_stage,
            });
        }
    }

    void ShaderModule::reflect_storage_buffers(spirv_cross::CompilerGLSL &glsl) {
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();
        size_t min_device_alignment = device().get_device_alignment();

        for(auto& resource : resources.storage_buffers) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);

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
            }

            Utils::StringId strId = INTERN_STR(name.c_str());
            auto padded_size = Buffer::pad_uniform_buffer_size(ubo_size);
            shader_resources.push_back(ShaderResource{strId, set, binding, padded_size, 1, 1, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (VkShaderStageFlags)shader_stage});
            combined_uniform_size += padded_size;
        }
    }

    void ShaderModule::reflect_uniform_buffers(spirv_cross::CompilerGLSL &glsl) {
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();
        size_t min_device_alignment = device().get_device_alignment();

        for (auto &resource : resources.uniform_buffers) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            std::string name = glsl.get_name(resource.id);

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
            }

            Utils::StringId strId = INTERN_STR(name.c_str());
            auto padded_size = Buffer::pad_uniform_buffer_size(ubo_size);
            shader_resources.push_back(ShaderResource{strId, set, binding, padded_size, 1, 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (VkShaderStageFlags)shader_stage});
            combined_uniform_size += padded_size;
        }
    }

    void ShaderModule::reflect_push_constants(spirv_cross::CompilerGLSL &glsl) {
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        for (auto &resource : resources.push_constant_buffers) {
            auto ranges = glsl.get_active_buffer_ranges(resource.id);
            std::string name = glsl.get_name(resource.id);

            logger::debug() << "Push constant name: " << name;

            VkPushConstantRange push_constant_range = {};

            const spirv_cross::SPIRType &type = glsl.get_type(resource.base_type_id);
            size_t push_size = glsl.get_declared_struct_size(type);
            logger::debug() << "Push constant size: " << push_size;
            if(type.basetype == spirv_cross::SPIRType::Struct) {
                size_t push_size = glsl.get_declared_struct_size(type);

                push_constant_range.offset = 0;
                push_constant_range.size = push_size;
                push_constant_range.stageFlags = shader_stage;

                push_constants_new[name] = push_constant_range;
            } else {
                for (auto &r : ranges) {
                    push_constant_range.offset = r.offset;
                    push_constant_range.size = r.range;
                    push_constant_range.stageFlags = shader_stage;

                    push_constants_new[name] = push_constant_range;
                }
            }
        }
    }
}