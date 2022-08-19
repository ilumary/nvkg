#include <nvkg/Renderer/Material/Material.hpp>
#include <nvkg/Renderer/Mesh/Mesh.hpp>
#include <nvkg/Renderer/Swapchain/Swapchain.hpp>
#include <nvkg/Renderer/Utils/Descriptor.hpp>

namespace nvkg {

    NVKGMaterial::NVKGMaterial() {
        vert_shader_new = nullptr;
        frag_shader_new = nullptr;
        shader_count = 0;
        is_new_shader_class = true;
        buffer_size = 0;
    }

    NVKGMaterial::NVKGMaterial(ShaderModule* vertex_shader, ShaderModule* fragment_shader) {
        NVKG_ASSERT(vertex_shader != nullptr && fragment_shader != nullptr, 
            "Error: the vertex and fragment shaders must be initialised");

        vert_shader_new = vertex_shader;
        frag_shader_new = fragment_shader;
        shader_count = 2;
        is_new_shader_class = true;

        buffer_size = vert_shader_new->combined_uniform_size + frag_shader_new->combined_uniform_size;
    }

    void NVKGMaterial::set_vert_shader_new(ShaderModule* shader) {
        if(vert_shader_new == nullptr) shader_count++;
        vert_shader_new = shader; 
        buffer_size += shader->combined_uniform_size;

        is_new_shader_class = true;
    }

    void NVKGMaterial::set_frag_shader_new(ShaderModule* shader) {
        if(frag_shader_new == nullptr) shader_count++;
        frag_shader_new = shader; 
        buffer_size += shader->combined_uniform_size;

        is_new_shader_class = true;
    }

    NVKGMaterial::~NVKGMaterial() {
        if (isFreed) return;

        destroy_material();
    }

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

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, desc_sets.size(), desc_sets.data(), desc_offs.size(), desc_offs.data());
    }

    void NVKGMaterial::push_constant(VkCommandBuffer command_buffer, size_t push_constant_size, const void* data) {
        vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, push_constant_size, data);
    }

    void NVKGMaterial::recreate_pipeline() {
        // Clear our graphics pipeline before swapchain re-creation
        pipeline.ClearPipeline();

        create_pipeline();
    }

    //TODO: rewrite for new shader module class
    void NVKGMaterial::create_pipeline() {
        std::vector<PipelineConfig::ShaderConfig> shader_configs{};

        if (vert_shader_new) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::VERTEX, vert_shader_new->shader_module });
        if (frag_shader_new) shader_configs.push_back(PipelineConfig::ShaderConfig { nullptr, PipelineConfig::PipelineStage::FRAGMENT, frag_shader_new->shader_module });

        size_t bindingCount = prop_vec.size();

        VkDescriptorSetLayout layouts[bindingCount];

        for(size_t i = 0; i < bindingCount; i++) {
            layouts[i] = prop_vec.at(i).descriptorBinding.layout;
        }

        std::vector<VkPushConstantRange> vpc{}, fpc{};

        if (vert_shader_new) vpc = vert_shader_new->push_constants;
        if (frag_shader_new) fpc = frag_shader_new->push_constants;

        push_constants.reserve(vpc.size() + fpc.size());
        push_constants.insert(push_constants.end(), vpc.begin(), vpc.end());
        push_constants.insert(push_constants.end(), fpc.begin(), fpc.end());

        std::cout << "\n";
        for(auto& pc : push_constants) {
            std::cout << "PUSH CONSTANT: " << pc.stageFlags << " " << pc.size << " " << pc.offset << std::endl;
        }

        void* push_constant_data;
        if(push_constants.size() == 0) { push_constant_data = nullptr; } else { push_constant_data = &push_constants[0]; }

        create_layout(layouts, bindingCount, (VkPushConstantRange*)push_constant_data, push_constants.size());

        NVKG_ASSERT(pipeline_layout != nullptr, "Cannot create pipeline without a valid layout!");
        
        // Maybe pipelineConfig should be a builder class?

        auto pipelineConfig = Pipeline::DefaultPipelineConfig();
        pipelineConfig.rasterizationInfo.polygonMode = (VkPolygonMode)shader_config.mode;
        pipelineConfig.inputAssemblyInfo.topology = (VkPrimitiveTopology)shader_config.topology;
        
        pipelineConfig.renderPass = SwapChain::GetInstance()->GetRenderPass()->GetRenderPass();
        pipelineConfig.pipelineLayout = pipeline_layout;
        
        pipelineConfig.vertexData = VertexDescription::CreateDescriptions(vert_count, vertex_binds.data());

        pipeline.RecreatePipeline(
            shader_configs.data(),
            shader_count,
            pipelineConfig
        );
    }

    // At some point it might be useful to batch create
    // bindings. To do this we'd need to sort all bindings by type, stage, and 
    // binding. 
    
    // This could potentially break if we have multiple 
    // uniforms in the same dynamic uniform or storage buffer. 
    void NVKGMaterial::init_descriptors() {
        auto device = VulkanDevice::get_device_instance();

        auto descriptorPool = DescriptorPool::get_descr_pool();

        // Create a descriptor set for our object transforms.

        // A layout binding must be created for each resource used by a shader. 
        // If we have two uniforms at different bindings (binding 0, 1, 2...etc), then we need
        // a new layout binding for each one. 

        size_t prop_count = prop_vec.size();

        VkDescriptorBufferInfo buff_infos[prop_count]; //TODO replace with vector
        VkWriteDescriptorSet write_desc_sets[prop_count]; //TODO replace with vector

        std::cout << "Allocating descriptor set storage of " << prop_count << std::endl;

        for (size_t i = 0; i < prop_count; i++) {
            auto& property = prop_vec.at(i);
            auto& binding = property.descriptorBinding;

            // TODO: bindings MUST be unique for all bound shaders. This means that
            // TODO: new bindings on a new shader must follow a consecutive order.
            auto layoutBinding = Utils::Descriptor::CreateLayoutBinding(
                property.binding, 
                1, 
                binding.type,
                property.stage
            );

            auto stage = property.stage == VK_SHADER_STAGE_VERTEX_BIT ? "vertex" : 
                property.stage == VK_SHADER_STAGE_FRAGMENT_BIT ? "fragment" : "unknown";

            std::cout << "Creating a layout binding for binding " << property.binding << " at stage: " << stage << std::endl;

            // Create all layouts
            
            NVKG_ASSERT(Utils::Descriptor::CreateLayout(device->device(), OUT binding.layout, &layoutBinding, 1),
            "Failed to create descriptor set!");

            uint64_t offset = property.offset;

            buff_infos[i] = Utils::Descriptor::CreateBufferInfo(buffer.buffer, offset, property.size * property.count);
            std::cout << "Property Size: " << property.size * property.count << std::endl;

            std::cout << "Allocating descriptor set for binding " << property.binding << std::endl;
            Utils::Descriptor::AllocateSets(device->device(), &binding.descriptorSet, descriptorPool, 1, &binding.layout);

            desc_sets.push_back(binding.descriptorSet);
            
            write_desc_sets[i] = (
                Utils::Descriptor::CreateBufferWriteSet(
                    property.binding, 
                    binding.descriptorSet, 
                    1, 
                    (VkDescriptorType)binding.type,
                    &buff_infos[i]
            ));
        }

        std::cout << "Successfully created all required layouts!" << std::endl;

        std::cout << "Total descriptor sets: " <<  desc_sets.size() << std::endl;

        Utils::Descriptor::WriteSets(device->device(), &write_desc_sets[0], prop_count);
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
        auto uniforms = shader->uniforms;

        for(auto& uniform : uniforms) {
            if (has_prop(uniform.id)) {
                std::cout << "Property already exists!" << std::endl;
                auto& property = get_prop(uniform.id);
                property.stage = property.stage | (VkShaderStageFlags) shader->shader_stage;
                continue;
            };

            Property property = {
                uniform.binding, 
                uniform.id, 
                (VkShaderStageFlags)shader->shader_stage, 
                offset,  
                uniform.size * uniform.arraySize,
                uniform.dyn_count
            };

            property.descriptorBinding = { VK_NULL_HANDLE, VK_NULL_HANDLE, uniform.type };
            prop_vec.push_back(property);

            std::cout << "Added new uniform to binding: " << uniform.binding << std::endl;

            std::cout << "Added new property of size: " << uniform.size << " with buffer offset: " << offset << std::endl;

            offset += (uniform.size * uniform.arraySize) * uniform.dyn_count;
        }
    }
    
    void NVKGMaterial::destroy_material() {
        auto device = VulkanDevice::get_device_instance();

        for (auto& property : prop_vec) {
            vkDestroyDescriptorSetLayout(device->device(), property.descriptorBinding.layout, nullptr);
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
        for(auto& property : prop_vec) {
            if (id == property.id) {
                Buffer::copy_data(buffer, dataSize, data, property.offset);
                return;
            }
        }
    }

    bool NVKGMaterial::has_prop(Utils::StringId id) {
        for(auto& property : prop_vec) {
            if (id == property.id) {
                return true;
            }
        }
        return false;
    }

    void NVKGMaterial::set_uniform_data(const char* name, VkDeviceSize dataSize, const void* data) {
        auto id = INTERN_STR(name);

        for(auto& property : prop_vec) {
            if (id == property.id) {
                Buffer::copy_data(buffer, dataSize, data, property.offset);
                return;
            }
        }
    }

    void NVKGMaterial::set_texture(SampledTexture* tex, std::string tex_name, uint32_t binding, VkShaderStageFlagBits shader_stage) {
        
    }

    void NVKGMaterial::create_materials(std::initializer_list<NVKGMaterial*> materials) {
        for (auto material : materials) material->create_material();
    }

    NVKGMaterial::Property& NVKGMaterial::get_prop(Utils::StringId id) {
        for(auto& property : prop_vec) {
            if (id == property.id) return property;
        }

        NVKG_ASSERT(false, "No property with ID: " << id << " exists!");
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

        if (vert_shader_new && is_new_shader_class) {
            std::cout << "Using new vertex shader class" << std::endl;
            add_shader(vert_shader_new);
            set_shader_props(vert_shader_new, OUT offset);
        }

        if (frag_shader_new && is_new_shader_class) {
            std::cout << "Using new frag shader class" << std::endl;
            add_shader(frag_shader_new);
            set_shader_props(frag_shader_new, OUT offset);
        }
        
        std::cout << "Total properties: " << prop_vec.size() << std::endl;
        
        init_descriptors();

        create_pipeline();
        
        std::cout << "Built material with size: " << buffer_size << std::endl;

        std::cout << "\n";
    }
}
