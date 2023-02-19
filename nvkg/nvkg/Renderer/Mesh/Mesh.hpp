#pragma once

#include <nvkg/Renderer/Core.hpp>
#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Pipeline/PipelineConfig.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <array>

namespace nvkg {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct Vertex2D {
        glm::vec2 position;
        glm::vec2 uv;
        glm::vec4 color;
    };

    bool operator==(const Vertex& left, const Vertex& right);
    bool operator==(const Vertex2D& left, const Vertex2D& right);

    struct staged_buffer {
        Buffer::Buffer staging_buffer_, buffer_;
        VkBufferUsageFlagBits buffer_usage_;

        staged_buffer(VkBufferUsageFlagBits buffer_usage) {
            buffer_usage_ = buffer_usage;
        }

        ~staged_buffer() {
            Buffer::destroy_buffer(staging_buffer_);
            Buffer::destroy_buffer(buffer_);
        }

        void init_staging_buffer(VkDeviceSize size) {
            Buffer::create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                staging_buffer_.buffer, staging_buffer_.bufferMemory);
        }

        void create_buffer(const void* data, std::size_t size) {
            if(size == 0) {
                logger::debug(logger::Level::Error) << "Tried creating buffer with zero data";
                return;
            }

            Buffer::copy_data(staging_buffer_, size, data);

            Buffer::create_buffer(
                size,
                buffer_usage_ | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                // specifies that data is accessible on the CPU.
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                OUT buffer_.buffer,
                OUT buffer_.bufferMemory
            );

            Buffer::copy_buffer(staging_buffer_.buffer, buffer_.buffer, size);
        }

        void update(const void* data, std::size_t size) {
            if(size == 0) {
                logger::debug(logger::Level::Error) << "Tried updating buffer with zero data";
                return;
            } 

            Buffer::copy_data(staging_buffer_, size, data);
            Buffer::copy_buffer(staging_buffer_.buffer, buffer_.buffer, size);
        }
    };

    class Mesh {
        public:

            struct MeshData {
                uint64_t vertexSize {0};
                const void* vertices {nullptr};
                uint32_t vertexCount {0}; 
                uint32_t* indices {nullptr}; 
                uint32_t indexCount {0};
            };

            Mesh();
            Mesh(const MeshData& meshData);
            ~Mesh();

            Mesh(const Mesh&) = delete;
            void operator=(const Mesh& other) = delete;

            void load_vertices(const Mesh::MeshData& meshData);
            void update_vertices(const Mesh::MeshData& meshData);

            void bind(VkCommandBuffer commandBuffer, uint32_t bind_id = 0);

            bool has_index_buffer() { return has_index_buffer_; }

            uint32_t get_vertex_count() { return vertex_count_; }
            uint32_t get_index_count() { return index_count_; }

            staged_buffer vertex_buffer_;
            staged_buffer index_buffer_;

        private:

            bool has_index_buffer_ = false, has_vertex_buffer_ = false;

            uint32_t index_count_ = 0, vertex_count_ = 0;
    };
}
