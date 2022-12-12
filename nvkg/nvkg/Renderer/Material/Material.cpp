#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Mesh/Mesh.hpp>
#include <nvkg/Renderer/Swapchain/Swapchain.hpp>
#include <nvkg/Renderer/Utils/Descriptor.hpp>

namespace nvkg {

    Material::Material(MaterialConfig config) {
        config_ = config;
        for(auto& shader : config_.shaders) {
            std::unique_ptr<ShaderModule> tmp = std::make_unique<ShaderModule>(shader, true);
            shaders[tmp->shader_stage] = std::move(tmp);
        }

        for(auto& texture : config_.textures) {
            set_texture(texture.second, texture.first);
        }

        create_material();
    }

    Material::~Material() {
        auto device = VulkanDevice::get_device_instance();

        for (auto& layout : descriptor_set_layouts) {
            vkDestroyDescriptorSetLayout(device->device(), layout, nullptr);
        }

        pipeline.destroy();
        
        vkDestroyPipelineLayout(device->device(), pipeline_layout, nullptr);
        
        Buffer::destroy_buffer(buffer);
    }

    void Material::bind(VkCommandBuffer commandBuffer) {
        pipeline.bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);
    }

    void Material::push_constant(VkCommandBuffer command_buffer, std::string name, size_t push_constant_size, const void* data) {
        if(push_constants.count(name) > 0) {
            const auto& pc = push_constants[name]; 
            vkCmdPushConstants(command_buffer, pipeline_layout, pc.stageFlags, 0, push_constant_size, data);
        }
    }

    void Material::set_texture(SampledTexture* tex, std::string tex_name) {
        auto id = INTERN_STR(tex_name.c_str());
        logger::debug() << "Added texture with id " << id << " to material";
        textures[id] = tex;
    }

    void Material::prepare_desc_set_layouts() {
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

        for(const auto& [stage, shader] : shaders) {
            for(const auto& [name, push_constant_range] : shader->push_constants_new) {
                if(push_constants.count(name) > 0) {
                    push_constants[name].stageFlags = push_constants[name].stageFlags | push_constant_range.stageFlags; 
                    continue;
                }
                push_constants[name] = push_constant_range;
            }
        }

        std::vector<VkPushConstantRange> tmp_push_constants(push_constants.size());

        for(const auto& pc : push_constants) {
            tmp_push_constants.push_back(pc.second);
        }

        void* push_constant_data;
        if(tmp_push_constants.size() == 0) { push_constant_data = nullptr; } else { push_constant_data = &tmp_push_constants[0]; }

        PipelineConfig::create_pipeline_layout(
            VulkanDevice::get_device_instance()->device(),
            OUT &pipeline_layout, 
            &descriptor_set_layouts[0], 
            descriptor_set_layouts.size(), 
            (VkPushConstantRange*)push_constant_data, 
            tmp_push_constants.size()
        );

        NVKG_ASSERT(pipeline_layout != nullptr, "Cannot create pipeline without a valid layout!");
    }

    void Material::prepare_pipeline() { 
        std::vector<PipelineConfig::ShaderConfig> shader_configs{};

        shader_configs.push_back(PipelineConfig::ShaderConfig { VK_SHADER_STAGE_VERTEX_BIT, shaders[VK_SHADER_STAGE_VERTEX_BIT]->shader_module });
        shader_configs.push_back(PipelineConfig::ShaderConfig { VK_SHADER_STAGE_FRAGMENT_BIT, shaders[VK_SHADER_STAGE_FRAGMENT_BIT]->shader_module });

        PipelineInit pipeline_conf = Pipeline::default_pipeline_init();
        
        if(config_.pipeline_configurator) { config_.pipeline_configurator(pipeline_conf); } //user specified changes to pipeline

        pipeline_conf.render_pass = SwapChain::GetInstance()->GetRenderPass()->GetRenderPass();
        pipeline_conf.pipeline_layout = pipeline_layout;
        pipeline_conf.vertex_data = VertexDescription::CreateDescriptions(vertex_binds.size(), vertex_binds.data());

        pipeline.create_graphics_pipeline(shader_configs.data(), shaders.size(), pipeline_conf);
    }

    void Material::setup_descriptor_sets() {
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
                        .sampler = textures[prop.id]->sampler->sampler,
                        .imageView = textures[prop.id]->image_view->image_view,
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

    void Material::add_shader(const std::unique_ptr<ShaderModule>& shader) {
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

    void Material::set_shader_props(const std::unique_ptr<ShaderModule>& shader, uint64_t& offset, uint16_t& res_counter) {
        auto shader_resources = shader->shader_resources;

        for(auto& res : shader_resources) {
            if (has_res(res.id)) {
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

            offset = buffer_size += (res.size * res.arraySize) * res.dyn_count;

            res_counter++;
        }
    }

    void Material::set_uniform_data(Utils::StringId id, VkDeviceSize dataSize, const void* data) {
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

    bool Material::has_res(Utils::StringId id) {
        for(const auto& [k, v] : resources_per_set) {
            for(auto& prop : v) {
                if(prop.id == id) return true;
            }
        }

        return false;
    }

    ShaderResource& Material::get_res(Utils::StringId id) {
        for(auto& [k, v] : resources_per_set) {
            for(auto& prop : v) {
                if(prop.id == id) return prop;
            }
        }

        NVKG_ASSERT(false, "No property with ID: " << id << " exists!");
    }

    void Material::set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data) {
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

    void Material::create_material() {
        logger::debug() << "CREATING NEW MATERIAL";

        uint64_t offset = 0;
        uint16_t counter = 0;

        for(const auto& [stage, shader] : shaders) {
            add_shader(shader);
            set_shader_props(shader, OUT offset, counter);
        }

        if(buffer_size > 0) {
            Buffer::create_buffer(
                buffer_size,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                OUT buffer.buffer,
                OUT buffer.bufferMemory
            );
        }

        logger::debug() << "Retrieved " << resources_per_set.size() << " resources from shaders";

        prepare_desc_set_layouts();
        prepare_pipeline();
        setup_descriptor_sets();
        
        logger::debug() << "Built material with size: " << buffer_size;
    }
}
