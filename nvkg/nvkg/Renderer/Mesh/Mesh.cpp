#include <nvkg/Renderer/Mesh/Mesh.hpp>

namespace nvkg {
    bool operator==(const Vertex& left, const Vertex& right) {
        return left.position == right.position && left.color == right.color 
            && left.normal == right.normal && left.uv == right.uv;
    }

    bool operator==(const Vertex2D& left, const Vertex2D& right) {
        return left.color == right.color && left.position == right.position && left.uv == right.uv;
    }

    Mesh::Mesh() {}

    Mesh::Mesh(const MeshData& meshData) {
        load_vertices(meshData);
    }

    Mesh::~Mesh() {
        Buffer::destroy_buffer(vertex_staging_buffer_);
        Buffer::destroy_buffer(index_staging_buffer_);

        Buffer::destroy_buffer(vertex_buffer_);
        if (has_index_buffer_) { Buffer::destroy_buffer(index_buffer_); }
    }

    void Mesh::load_vertices(const Mesh::MeshData& meshData) {
        vertex_count_ = meshData.vertexCount;
        index_count_ = meshData.indexCount;

        Buffer::create_buffer(
            meshData.vertexSize * meshData.vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            // Ensures that CPU and GPU memory are consistent across both devices.
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            OUT vertex_staging_buffer_.buffer,
            OUT vertex_staging_buffer_.bufferMemory);

        Buffer::create_buffer(
            sizeof(uint32_t) * meshData.indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            // Ensures that CPU and GPU memory are consistent across both devices.
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            OUT index_staging_buffer_.buffer,
            OUT index_staging_buffer_.bufferMemory);

        has_vertex_buffer_ = meshData.vertexCount > 0;

        if (has_vertex_buffer_) create_vertex_buffer(meshData.vertices, (meshData.vertexSize * meshData.vertexCount));

        has_index_buffer_ = meshData.indexCount > 0;

        if (has_index_buffer_) create_index_buffer(meshData.indices, (sizeof(uint32_t) * meshData.indexCount));
    }

    void Mesh::create_vertex_buffer(const void* vertices, size_t size) {
        Buffer::copy_data(vertex_staging_buffer_, size, vertices);

        Buffer::create_buffer(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            OUT vertex_buffer_.buffer,
            OUT vertex_buffer_.bufferMemory
        );

        Buffer::copy_buffer(vertex_staging_buffer_.buffer, vertex_buffer_.buffer, size);
    }

    void Mesh::create_index_buffer(const uint32_t* indices, size_t size) {
        Buffer::copy_data(index_staging_buffer_, size, indices);

        Buffer::create_buffer(
            size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            // specifies that data is accessible on the CPU.
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            index_buffer_.buffer,
            index_buffer_.bufferMemory
        );

        Buffer::copy_buffer(index_staging_buffer_.buffer, index_buffer_.buffer, size);
    }

    void Mesh::bind(VkCommandBuffer commandBuffer) {
        if (vertex_count_ > 0) {
            VkBuffer buffers[] = {vertex_buffer_.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }

        if (has_index_buffer_) vkCmdBindIndexBuffer(commandBuffer, index_buffer_.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::update_vertices(const Mesh::MeshData& meshData) {
        vertex_count_ = meshData.vertexCount;
        index_count_ = meshData.indexCount;

        update_vertex_buffer(meshData.vertices, (meshData.vertexSize * meshData.vertexCount));

        update_index_buffer(meshData.indices, (sizeof(uint32_t) * meshData.indexCount));
    }

    void Mesh::update_vertex_buffer(const void* vertices, size_t size) {
        if (size == 0) return;

        if (!has_vertex_buffer_) {
            create_vertex_buffer(vertices, size);
            has_vertex_buffer_ = true;
            return;
        }

        Buffer::copy_data(vertex_staging_buffer_, size, vertices);
        Buffer::copy_buffer(vertex_staging_buffer_.buffer, vertex_buffer_.buffer, size);
    }

    void Mesh::update_index_buffer(uint32_t* indices, size_t size) {
        if (size == 0) return;

        if (!has_index_buffer_) {
            create_index_buffer(indices, size);
            has_index_buffer_ = true;
            return;
        }

        Buffer::copy_data(index_staging_buffer_, size, indices);
        Buffer::copy_buffer(index_staging_buffer_.buffer, index_buffer_.buffer, size);
    }
}
