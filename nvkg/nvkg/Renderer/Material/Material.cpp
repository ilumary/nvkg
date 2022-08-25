#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Mesh/Mesh.hpp>
#include <nvkg/Renderer/Swapchain/Swapchain.hpp>
#include <nvkg/Renderer/Utils/Descriptor.hpp>

namespace nvkg {

    NVKGMaterial::NVKGMaterial() {
        vert_shader = nullptr;
        frag_shader = nullptr;
        shader_count = 0;
        buffer_size = 0;
    }

    NVKGMaterial::NVKGMaterial(ShaderModule* vertex_shader, ShaderModule* fragment_shader) {
        NVKG_ASSERT(vertex_shader != nullptr && fragment_shader != nullptr, 
            "Error: the vertex and fragment shaders must be initialised");

        vert_shader = vertex_shader;
        frag_shader = fragment_shader;
        shader_count = 2;

        buffer_size = vert_shader->combined_uniform_size + frag_shader->combined_uniform_size;
    }

    void NVKGMaterial::set_vert_shader_new(ShaderModule* shader) {
        if(vert_shader == nullptr) shader_count++;
        vert_shader = shader; 
        buffer_size += shader->combined_uniform_size;
    }

    void NVKGMaterial::set_frag_shader_new(ShaderModule* shader) {
        if(frag_shader == nullptr) shader_count++;
        frag_shader = shader; 
        buffer_size += shader->combined_uniform_size;
    }

    NVKGMaterial::~NVKGMaterial() {
        if (isFreed) return;

        destroy_material();
    }

    //TODO remove and replace by somehting les spaghetti like
    void NVKGMaterial::create_layout( VkDescriptorSetLayout* layouts, uint32_t layoutCount, VkPushConstantRange* pushConstants, uint32_t pushConstantCount) {
        auto device = VulkanDevice::get_device_instance();

        PipelineConfig::CreatePipelineLayout(
            device->device(), 
            OUT &pipeline_layout, 
            layouts, 
            layoutCount, 
            pushConstants, 
            pushConstantCount);
    }

    void NVKGMaterial::bind(VkCommandBuffer commandBuffer) {
        pipeline.bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);
    }

    void NVKGMaterial::push_constant(VkCommandBuffer command_buffer, size_t push_constant_size, const void* data) {
        vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, push_constant_size, data);
    }

    void NVKGMaterial::recreate_pipeline() {
        // Clear our graphics pipeline before swapchain re-creation
        pipeline.ClearPipeline();
        prepare_pipeline();
    }

    void NVKGMaterial::set_texture(SampledTexture* tex, std::string tex_name, VkShaderStageFlagBits shader_stage) {
        //TODO
    }

    void NVKGMaterial::prepare_desc_set_layouts() {
        //TODO add security checks for empty sets, no sets etc, also check for maximum number of sets

        descriptor_set_layouts = std::vector<VkDescriptorSetLayout>(max_set + 1);

        for(int i = 0; i < (max_set + 1); i++) {
            if(!res_sorted_by_set[i].empty()) {
                std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};

                for(auto& prop : res_sorted_by_set[i]) {
                    set_layout_bindings.push_back(nvkg::descriptors::descriptor_set_layout_binding(
                        prop.type,
                        prop.stage,
                        prop.binding
                    ));

                    std::cout << "Set Layout Binding for set " << i << ", binding " << prop.binding << ", stage " << prop.stage << ", type " << prop.type << std::endl;
                }

                std::cout << "Creating Set Layout " << i << " with " << set_layout_bindings.size() << " bindings" << std::endl;

                VkDescriptorSetLayoutCreateInfo descriptor_layout =
                    nvkg::descriptors::descriptor_set_layout_create_info(
                        set_layout_bindings.data(),
                        static_cast<uint32_t>(set_layout_bindings.size()));

                NVKG_ASSERT(vkCreateDescriptorSetLayout(VulkanDevice::get_device_instance()->device(), &descriptor_layout, nullptr, &descriptor_set_layouts[i]) == VK_SUCCESS, "Failed to create descriptor set layout");
            } //TODO check for possibility that inbetween sets are empty
        }

        std::vector<VkPushConstantRange> vpc{}, fpc{};

        if (vert_shader) vpc = vert_shader->push_constants;
        if (frag_shader) fpc = frag_shader->push_constants;

        push_constants.reserve(vpc.size() + fpc.size());
        push_constants.insert(push_constants.end(), vpc.begin(), vpc.end());
        push_constants.insert(push_constants.end(), fpc.begin(), fpc.end());

        void* push_constant_data;
        if(push_constants.size() == 0) { push_constant_data = nullptr; } else { push_constant_data = &push_constants[0]; }

        create_layout(&descriptor_set_layouts[0], descriptor_set_layouts.size(), (VkPushConstantRange*)push_constant_data, push_constants.size());

        NVKG_ASSERT(pipeline_layout != nullptr, "Cannot create pipeline without a valid layout!");
    }

    void NVKGMaterial::prepare_pipeline() { 
        std::vector<PipelineConfig::ShaderConfig> shader_configs{};

        if (vert_shader) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::VERTEX, vert_shader->shader_module });
        if (frag_shader) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::FRAGMENT, frag_shader->shader_module });

        auto pipelineConfig = Pipeline::DefaultPipelineConfig();
        pipelineConfig.rasterizationInfo.polygonMode = (VkPolygonMode)shader_config.mode;
        pipelineConfig.inputAssemblyInfo.topology = (VkPrimitiveTopology)shader_config.topology;
        
        pipelineConfig.renderPass = SwapChain::GetInstance()->GetRenderPass()->GetRenderPass();
        pipelineConfig.pipelineLayout = pipeline_layout;
        
        pipelineConfig.vertexData = VertexDescription::CreateDescriptions(vert_count, vertex_binds.data());

        pipeline.recreate_pipeline(
            shader_configs.data(),
            shader_count,
            pipelineConfig
        );
    }

    void NVKGMaterial::setup_descriptor_sets() {

        descriptor_sets = std::vector<VkDescriptorSet>(max_set + 1);

        VkDescriptorSetAllocateInfo alloc_info =
			nvkg::descriptors::descriptor_set_allocate_info(
				DescriptorPool::get_descr_pool(),
				&descriptor_set_layouts[0],
				descriptor_set_layouts.size());

        vkAllocateDescriptorSets(VulkanDevice::get_device_instance()->device(), &alloc_info, &descriptor_sets[0]);

        std::vector<VkWriteDescriptorSet> write_sets{};
        VkDescriptorBufferInfo descriptor_buffer_infos[10]; // TODO: find limit on descriptor_buffer_infos from shader
        int buf_counter = 0;

        for(auto& v : res_sorted_by_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                //if(prop.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    //image    
                //} else {
                    //storage/uniform
                    descriptor_buffer_infos[buf_counter] = {
                        .buffer = buffer.buffer,
                        .offset = prop.offset,
                        .range = prop.size * prop.dyn_count,
                    };

                    std::cout << "Prop Id: " << prop.id << std::endl;
                    std::cout << "Buffer Info: offset: " << descriptor_buffer_infos[buf_counter].offset << ", range: " << descriptor_buffer_infos[buf_counter].range << std::endl;

                    write_sets.push_back(
                        nvkg::descriptors::write_descriptor_set(
                            descriptor_sets[prop.set],
                            prop.type,
                            prop.binding,
                            &descriptor_buffer_infos[buf_counter]
                        )
                    );

                    buf_counter++;
                //}
            }
        }

        vkUpdateDescriptorSets(VulkanDevice::get_device_instance()->device(), static_cast<uint32_t>(write_sets.size()), write_sets.data(), 0, NULL);
    }

    void NVKGMaterial::add_shader(ShaderModule* shader) {
        auto& vertices = shader->vertex_bindings;

        vert_count += vertices.size();

        for(size_t i = 0; i < vertices.size(); i++) {
            auto& binding = vertices.at(i);
            auto& attributes = binding.attributes;

            if (attributes.size() == 0) continue;

            vertex_binds.push_back(
                VertexDescription::CreateBinding(
                    i, 
                    binding.vertexStride, 
                    VertexDescription::VERTEX,
                    attributes.data(),
                    attributes.size()
            ));
        }
    }

    void NVKGMaterial::set_shader_props(ShaderModule* shader, uint64_t& offset) {
        auto shader_resources = shader->shader_resources;

        for(auto& res : shader_resources) {
            if (has_prop(res.id)) {
                auto& property = get_res(res.id);
                property.stage = property.stage | (VkShaderStageFlags) shader->shader_stage;
                continue;
            };

            NVKG_ASSERT(res.set < MAX_DESCRIPTOR_SETS, "Aborting with invalid descriptor set count...");
            NVKG_ASSERT(res.binding < MAX_DESCRIPTOR_BINDINGS_PER_SET, "Aborting with invalid descriptor binding...");

            res.offset = offset;

            res_sorted_by_set[res.set].push_back(res);

            offset += (res.size * res.arraySize) * res.dyn_count;

            if(res.set > max_set) max_set = res.set;
        }
    }
    
    void NVKGMaterial::destroy_material() {
        auto device = VulkanDevice::get_device_instance();

        for (auto& layout : descriptor_set_layouts) {
            vkDestroyDescriptorSetLayout(device->device(), layout, nullptr);
        }

        pipeline.DestroyPipeline();
        
        vkDestroyPipelineLayout(device->device(), pipeline_layout, nullptr);
        
        Buffer::destroy_buffer(buffer);

        isFreed = true;
    }

    void NVKGMaterial::set_uniform_data(VkDeviceSize dataSize, const void* data) {
        Buffer::copy_data(buffer, dataSize, data);
    }

    void NVKGMaterial::set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data) {
        for(auto& v : res_sorted_by_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                if(prop.id == id) {
                    Buffer::copy_data(buffer, dataSize, data, prop.offset);
                    return;
                }
            }
        }

        std::cout << "Error while setting uniform data" << std::endl;
    }

    bool NVKGMaterial::has_prop(Utils::StringId id) {
        for(auto& v : res_sorted_by_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                if(prop.id == id) return true;
            }
        }

        return false;
    }

    ShaderResource& NVKGMaterial::get_res(Utils::StringId id) {
        for(auto& v : res_sorted_by_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                if(prop.id == id) return prop;
            }
        }

        NVKG_ASSERT(false, "No property with ID: " << id << " exists!");
    }

    void NVKGMaterial::set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data) {
        auto id = INTERN_STR(name);

        for(auto& v : res_sorted_by_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                if(prop.id == id) {
                    Buffer::copy_data(buffer, dataSize, data, prop.offset);
                    return;
                }
            }
        }

        std::cout << "Error while setting uniform data" << std::endl;
    }

    void NVKGMaterial::create_materials(std::initializer_list<NVKGMaterial*> materials) {
        for (auto material : materials) material->create_material();
    }


    void NVKGMaterial::create_material() {
        // Allocate buffer which can store all the data we need
        std::cout << "\nCREATING NEW MATERIAL" << std::endl;

        Buffer::create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            OUT buffer.buffer,
            OUT buffer.bufferMemory
        );

        uint64_t offset = 0;

        if (vert_shader) {
            add_shader(vert_shader);
            set_shader_props(vert_shader, OUT offset);
        }

        if (frag_shader) {
            add_shader(frag_shader);
            set_shader_props(frag_shader, OUT offset);
        }

        prepare_desc_set_layouts();
        prepare_pipeline();
        setup_descriptor_sets();
        
        std::cout << "Built material with size: " << buffer_size << std::endl;

        std::cout << "\n";
    }
}
