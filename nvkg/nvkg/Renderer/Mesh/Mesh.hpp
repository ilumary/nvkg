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

            void bind(VkCommandBuffer commandBuffer);

            bool has_index_buffer() { return has_index_buffer_; }

            uint32_t get_vertex_count() { return vertex_count_; }
            uint32_t get_index_count() { return index_count_; }

        private:

            void update_vertex_buffer(const void* vertices, size_t size);
            void update_index_buffer(uint32_t* indices, size_t size);

            void create_vertex_buffer(const void* vertices, size_t size);
            void create_index_buffer(const uint32_t* indices, size_t size);

            Buffer::Buffer vertex_staging_buffer_, index_staging_buffer_;

            Buffer::Buffer vertex_buffer_, index_buffer_;

            bool has_index_buffer_ = false, has_vertex_buffer_ = false;

            uint32_t index_count_ = 0, vertex_count_ = 0;
    };
}
