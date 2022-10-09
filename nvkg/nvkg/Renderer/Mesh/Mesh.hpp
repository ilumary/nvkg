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

            static constexpr size_t MAX_VERTICES = 10000;
            static constexpr size_t MAX_INDICES = 100000;

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

            void LoadVertices(const Mesh::MeshData& meshData);
            void UpdateVertices(const Mesh::MeshData& meshData);

            void UpdateVertexBuffer(const void* vertices);
            void UpdateIndexBuffer(uint32_t* indices);

            void Bind(VkCommandBuffer commandBuffer);
            void DestroyMesh();

            bool HasIndexBuffer() { return hasIndexBuffer; }

            uint32_t GetVertexCount() { return vertexCount; }
            uint32_t GetIndexCount() { return indexCount; }

        private:

            void CreateVertexBuffers(const void* vertices);
            void CreateIndexBuffer(const uint32_t* indices);

            Buffer::Buffer globalStagingBuffer;
            Buffer::Buffer globalIndexStagingBuffer;

            Buffer::Buffer vertexBuffer;
            Buffer::Buffer indexBuffer;

            bool hasIndexBuffer = false;
            bool hasVertexBuffer = false;

            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;

            uint64_t indexSize {sizeof(uint32_t)};
            uint64_t vertexSize = 0;

            bool isFreed = false;
    };
}
