#include <nvkg/Renderer/Mesh/Mesh.hpp>

namespace nvkg {
    bool operator==(const Vertex& left, const Vertex& right) {
        return left.position == right.position && left.color == right.color 
            && left.normal == right.normal && left.uv == right.uv;
    }

    bool operator==(const Vertex2D& left, const Vertex2D& right) {
        return left.color == right.color && left.position == right.position;
    }

    Mesh::Mesh() {}

    Mesh::Mesh(const MeshData& meshData) {
        LoadVertices(meshData);
    }

    Mesh::~Mesh() {
        if (isFreed) return;
        DestroyMesh();
    }

    void Mesh::LoadVertices(const Mesh::MeshData& meshData) {
        vertexCount = meshData.vertexCount;
        indexCount = meshData.indexCount;
        vertexSize = meshData.vertexSize;

        VkDeviceSize bufferSize = vertexSize * MAX_VERTICES;

        Buffer::create_buffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            // Ensures that CPU and GPU memory are consistent across both devices.
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            OUT globalStagingBuffer.buffer,
            OUT globalStagingBuffer.bufferMemory);

        Buffer::create_buffer(
            indexSize * MAX_INDICES,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            // Ensures that CPU and GPU memory are consistent across both devices.
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            OUT globalIndexStagingBuffer.buffer,
            OUT globalIndexStagingBuffer.bufferMemory);

        hasVertexBuffer = vertexCount > 0;

        if (hasVertexBuffer) CreateVertexBuffers(meshData.vertices);

        hasIndexBuffer = indexCount > 0;

        if (hasIndexBuffer) CreateIndexBuffer(meshData.indices);
    }

    void Mesh::DestroyMesh() {
        Buffer::destroy_buffer(vertexBuffer);
        Buffer::destroy_buffer(globalStagingBuffer);
        Buffer::destroy_buffer(globalIndexStagingBuffer);

        if (hasIndexBuffer) {
            Buffer::destroy_buffer(indexBuffer);
        }
        
        isFreed = true;
    }

    void Mesh::CreateVertexBuffers(const void* vertices) {
        VkDeviceSize bufferSize = vertexSize * MAX_VERTICES;

        Buffer::copy_data(globalStagingBuffer, vertexSize * vertexCount, vertices);

        Buffer::create_buffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            OUT vertexBuffer.buffer,
            OUT vertexBuffer.bufferMemory
        );

        Buffer::copy_buffer(globalStagingBuffer.buffer, vertexBuffer.buffer, bufferSize);
    }

    void Mesh::CreateIndexBuffer(const uint32_t* indices) {
        VkDeviceSize bufferSize = indexSize * MAX_INDICES;

        Buffer::copy_data(globalIndexStagingBuffer, indexSize * indexCount, indices);

        Buffer::create_buffer(
                bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBuffer.buffer,
            indexBuffer.bufferMemory
        );

        Buffer::copy_buffer(globalIndexStagingBuffer.buffer, indexBuffer.buffer, bufferSize);
    }

    void Mesh::Bind(VkCommandBuffer commandBuffer) {
        if (vertexCount > 0) {
            VkBuffer buffers[] = {vertexBuffer.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }

        if (hasIndexBuffer) vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::UpdateVertices(const Mesh::MeshData& meshData) {
        vertexCount = meshData.vertexCount;
        indexCount = meshData.indexCount;
        vertexSize = meshData.vertexSize;

        UpdateVertexBuffer(meshData.vertices);

        UpdateIndexBuffer(meshData.indices);
    }

    void Mesh::UpdateVertexBuffer(const void* vertices) {
        if (vertexCount == 0) return;

        if (!hasVertexBuffer) {
            CreateVertexBuffers(vertices);
            hasVertexBuffer = true;
            return;
        }

        Buffer::copy_data(globalStagingBuffer, vertexSize * vertexCount, vertices);
        Buffer::copy_buffer(globalStagingBuffer.buffer, vertexBuffer.buffer, vertexSize * vertexCount);
    }

    void Mesh::UpdateIndexBuffer(uint32_t* indices) {
        if (indexCount == 0) return;

        if (!hasIndexBuffer) {
            CreateIndexBuffer(indices);
            hasIndexBuffer = true;
            return;
        }

        VkDeviceSize indexSize = sizeof(uint32_t) * indexCount;

        Buffer::copy_data(globalIndexStagingBuffer, indexSize, indices);
        Buffer::copy_buffer(globalIndexStagingBuffer.buffer, indexBuffer.buffer, indexSize);
    }
}
