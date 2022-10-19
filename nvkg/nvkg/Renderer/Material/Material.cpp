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

        PipelineConfig::create_pipeline_layout(
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

    void NVKGMaterial::push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data) {
        if(push_constants_new.count(name) > 0) {
            const auto& pc = push_constants_new[name]; 
            vkCmdPushConstants(command_buffer, pipeline_layout, pc.stageFlags, 0, push_constant_size, data);
        }
    }

    void NVKGMaterial::recreate_pipeline() {
        // Clear our graphics pipeline before swapchain re-creation
        pipeline.clear();
        prepare_pipeline();
    }

    void NVKGMaterial::set_texture(SampledTexture* tex, std::string tex_name, VkShaderStageFlagBits shader_stage) {
        auto id = INTERN_STR(tex_name.c_str());
        logger::debug() << "Added texture with id " << id << " to material";
        textures[id] = {id, shader_stage, tex};
    }

    void NVKGMaterial::prepare_desc_set_layouts() {
        descriptor_set_layouts = std::vector<VkDescriptorSetLayout>(resources_per_set.size());

        for(const auto& [si, res_vec] : resources_per_set) {
                std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};

                unsigned short set = (si & set_mask);
                unsigned short index = (si & index_mask) >> 8;

                for(auto& res : res_vec) {
                    set_layout_bindings.push_back(nvkg::descriptors::descriptor_set_layout_binding(
                        res.type,
                        res.stage,
                        res.binding
                    ));

                    logger::debug() << "Set Layout Binding for set " << set << ", binding " << res.binding << ", stage " << res.stage << ", type " << res.type;
                }

                logger::debug() << "Creating Set Layout " << set << " with " << set_layout_bindings.size() << " bindings and index " << index;

                VkDescriptorSetLayoutCreateInfo descriptor_layout =
                    nvkg::descriptors::descriptor_set_layout_create_info(
                        set_layout_bindings.data(),
                        static_cast<uint32_t>(set_layout_bindings.size()));

                NVKG_ASSERT(vkCreateDescriptorSetLayout(VulkanDevice::get_device_instance()->device(), &descriptor_layout, nullptr, &descriptor_set_layouts[index]) == VK_SUCCESS, "Failed to create descriptor set layout");
        }

        for(const auto& [s, v] : vert_shader->push_constants_new) {
            if(push_constants_new.count(s) > 0) {
                push_constants_new[s].stageFlags = push_constants_new[s].stageFlags | v.stageFlags; 
                continue;
            }
            push_constants_new[s] = v;
        }

        for(const auto& [s, v] : frag_shader->push_constants_new) {
            if(push_constants_new.count(s) > 0) {
                push_constants_new[s].stageFlags = push_constants_new[s].stageFlags | v.stageFlags; 
                continue;
            }
            push_constants_new[s] = v;
        }

        std::vector<VkPushConstantRange> tmp_push_constants(push_constants_new.size());

        for(const auto& pc : push_constants_new) {
            tmp_push_constants.push_back(pc.second);
        }

        void* push_constant_data;
        if(tmp_push_constants.size() == 0) { push_constant_data = nullptr; } else { push_constant_data = &tmp_push_constants[0]; }

        create_layout(&descriptor_set_layouts[0], descriptor_set_layouts.size(), (VkPushConstantRange*)push_constant_data, tmp_push_constants.size());

        NVKG_ASSERT(pipeline_layout != nullptr, "Cannot create pipeline without a valid layout!");
    }

    void NVKGMaterial::prepare_pipeline() { 
        std::vector<PipelineConfig::ShaderConfig> shader_configs{};

        if (vert_shader) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::VERTEX, vert_shader->shader_module });
        if (frag_shader) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::FRAGMENT, frag_shader->shader_module });

        PipelineInit pipeline_conf = Pipeline::default_pipeline_init();
        pipeline_conf.rasterization_state.polygonMode = (VkPolygonMode)shader_config.mode;
        pipeline_conf.input_assembly_state.topology = (VkPrimitiveTopology)shader_config.topology;
        pipeline_conf.render_pass = SwapChain::GetInstance()->GetRenderPass()->GetRenderPass();
        pipeline_conf.pipeline_layout = pipeline_layout;
        pipeline_conf.vertex_data = VertexDescription::CreateDescriptions(vertex_binds.size(), vertex_binds.data());

        pipeline.create_graphics_pipeline(shader_configs.data(), shader_count, pipeline_conf);
    }

    void NVKGMaterial::setup_descriptor_sets() {
        descriptor_sets = std::vector<VkDescriptorSet>(resources_per_set.size());

        VkDescriptorSetAllocateInfo alloc_info =
			nvkg::descriptors::descriptor_set_allocate_info(
				DescriptorPool::get_descr_pool(),
				&descriptor_set_layouts[0],
				descriptor_set_layouts.size());

        vkAllocateDescriptorSets(VulkanDevice::get_device_instance()->device(), &alloc_info, &descriptor_sets[0]);

        std::vector<VkWriteDescriptorSet> write_sets{};
        VkDescriptorBufferInfo descriptor_buffer_infos[10]; // TODO: find limit on descriptor_buffer_infos from shader
        VkDescriptorImageInfo descriptor_image_infos[10]; // TODO: find limit on descriptor_image_infos from shader

        int buf_counter = 0, img_counter = 0;

        for(const auto& [k, v] : resources_per_set) {

            //unsigned short set = (k & set_mask);
            unsigned short index = (k & index_mask) >> 8;
            
            for(auto& prop : v) {
                if(prop.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    //image
                    if (textures.find(prop.id) == textures.end()) {
                        logger::debug() << "Unable to find texture with id " << prop.id << ". Continuing...";
                        continue;
                    }

                    descriptor_image_infos[img_counter] = {
                        .sampler = textures[prop.id].texture->sampler->sampler,
                        .imageView = textures[prop.id].texture->image_view->image_view,
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    };

                    write_sets.push_back(
                        nvkg::descriptors::write_descriptor_set(
                            descriptor_sets[index],
                            prop.type,
                            prop.binding,
                            &descriptor_image_infos[img_counter]
                        )
                    );

                    img_counter++;

                } else {
                    //storage/uniform
                    descriptor_buffer_infos[buf_counter] = {
                        .buffer = buffer.buffer,
                        .offset = prop.offset,
                        .range = prop.size * prop.dyn_count,
                    };

                    logger::debug() << "Prop Id: " << prop.id;
                    logger::debug() << "Buffer Info: offset: " << descriptor_buffer_infos[buf_counter].offset << ", range: " << descriptor_buffer_infos[buf_counter].range;

                    write_sets.push_back(
                        nvkg::descriptors::write_descriptor_set(
                            descriptor_sets[index],
                            prop.type,
                            prop.binding,
                            &descriptor_buffer_infos[buf_counter]
                        )
                    );

                    buf_counter++;
                }
            }
        }

        vkUpdateDescriptorSets(VulkanDevice::get_device_instance()->device(), static_cast<uint32_t>(write_sets.size()), write_sets.data(), 0, NULL);
    }

    void NVKGMaterial::add_shader(ShaderModule* shader) {
        auto& vertices = shader->vertex_bindings;

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

    void NVKGMaterial::set_shader_props(ShaderModule* shader, uint64_t& offset, uint16_t& res_counter) {
        auto shader_resources = shader->shader_resources;

        for(auto& res : shader_resources) {
            if (has_prop(res.id)) {
                auto& property = get_res(res.id);
                property.stage = property.stage | (VkShaderStageFlags) shader->shader_stage;
                continue;
            };

            NVKG_ASSERT(res.set < MAX_DESCRIPTOR_SETS, "Aborting with invalid descriptor set count...");
            NVKG_ASSERT(res.binding < MAX_DESCRIPTOR_BINDINGS_PER_SET, "Aborting with invalid descriptor binding...");

            uint16_t map_index = 0;
            map_index = ((map_index & index_mask) | (res_counter << 8)) | ((map_index & set_mask) | res.set);

            res.offset = offset;

            resources_per_set[map_index].push_back(res);

            offset += (res.size * res.arraySize) * res.dyn_count;

            res_counter++;
        }
    }
    
    void NVKGMaterial::destroy_material() {
        auto device = VulkanDevice::get_device_instance();

        for (auto& layout : descriptor_set_layouts) {
            vkDestroyDescriptorSetLayout(device->device(), layout, nullptr);
        }

        pipeline.destroy();
        
        vkDestroyPipelineLayout(device->device(), pipeline_layout, nullptr);
        
        Buffer::destroy_buffer(buffer);

        isFreed = true;
    }

    void NVKGMaterial::set_uniform_data(VkDeviceSize dataSize, const void* data) {
        Buffer::copy_data(buffer, dataSize, data);
    }

    void NVKGMaterial::set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data) {
        for(const auto& [k, v] : resources_per_set) {
            for(auto& prop : v) {
                if(prop.id == id) {
                    Buffer::copy_data(buffer, dataSize, data, prop.offset);
                    return;
                }
            }
        }

        logger::debug() << "Error while setting uniform data";
    }

    bool NVKGMaterial::has_prop(Utils::StringId id) {
        for(const auto& [k, v] : resources_per_set) {
            for(auto& prop : v) {
                if(prop.id == id) return true;
            }
        }

        return false;
    }

    ShaderResource& NVKGMaterial::get_res(Utils::StringId id) {
        for(auto& [k, v] : resources_per_set) {
            for(auto& prop : v) {
                if(prop.id == id) return prop;
            }
        }

        NVKG_ASSERT(false, "No property with ID: " << id << " exists!");
    }

    void NVKGMaterial::set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data) {
        auto id = INTERN_STR(name);

        for(const auto& [k, v] : resources_per_set) {
            if(v.empty()) continue;
            for(auto& prop : v) {
                if(prop.id == id) {
                    Buffer::copy_data(buffer, dataSize, data, prop.offset);
                    return;
                }
            }
        }

        logger::debug(logger::Level::Error) << "Error while setting uniform data";
    }

    void NVKGMaterial::create_materials(std::initializer_list<NVKGMaterial*> materials) {
        for (auto material : materials) material->create_material();
    }

    void NVKGMaterial::create_material() {
        // Allocate buffer which can store all the data we need
        logger::debug() << "CREATING NEW MATERIAL";

        //TODO set buffer size after resource collection to prevent doubled allocations
        if(buffer_size > 0) {
            Buffer::create_buffer(
                buffer_size,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                OUT buffer.buffer,
                OUT buffer.bufferMemory
            );
        }

        uint64_t offset = 0;
        uint16_t counter = 0;

        if (vert_shader) {
            add_shader(vert_shader);
            set_shader_props(vert_shader, OUT offset, counter);
        }

        if (frag_shader) {
            add_shader(frag_shader);
            set_shader_props(frag_shader, OUT offset, counter);
        }

        logger::debug() << "Retrieved " << resources_per_set.size() << " resources from shaders";

        prepare_desc_set_layouts();
        prepare_pipeline();
        setup_descriptor_sets();
        
        logger::debug() << "Built material with size: " << buffer_size;
    }
}
