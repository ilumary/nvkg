#include <nvkg/Renderer/Mesh/Mesh.hpp>

namespace nvkg {
    bool operator==(const Vertex& left, const Vertex& right) {
        return left.position == right.position && left.color == right.color 
            && left.normal == right.normal && left.uv == right.uv;
    }

    bool operator==(const Vertex2D& left, const Vertex2D& right) {
        return left.color == right.color && left.position == right.position && left.uv == right.uv;
    }

    Mesh::Mesh()
        : vertex_buffer_(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), index_buffer_(VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {}

    Mesh::Mesh(const MeshData& meshData)
        : vertex_buffer_(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), index_buffer_(VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        load_vertices(meshData);
    }

    Mesh::~Mesh() {}

    void Mesh::load_vertices(const Mesh::MeshData& meshData) {
        vertex_count_ = meshData.vertexCount;
        index_count_ = meshData.indexCount;

        vertex_buffer_.init_staging_buffer(meshData.vertexSize * meshData.vertexCount);
        index_buffer_.init_staging_buffer(sizeof(uint32_t) * meshData.indexCount);

        has_vertex_buffer_ = meshData.vertexCount > 0;

        if(has_vertex_buffer_) vertex_buffer_.create_buffer(meshData.vertices, (meshData.vertexSize * meshData.vertexCount));

        has_index_buffer_ = meshData.indexCount > 0;

        if(has_index_buffer_) index_buffer_.create_buffer(meshData.indices, (sizeof(uint32_t) * meshData.indexCount));
    }

    void Mesh::bind(VkCommandBuffer commandBuffer) {
        if (vertex_count_ > 0) {
            VkBuffer buffers[] = {vertex_buffer_.buffer_.buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }

        if (has_index_buffer_) vkCmdBindIndexBuffer(commandBuffer, index_buffer_.buffer_.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::update_vertices(const Mesh::MeshData& meshData) {
        vertex_count_ = meshData.vertexCount;
        index_count_ = meshData.indexCount;

        vertex_buffer_.update(meshData.vertices, (meshData.vertexSize * meshData.vertexCount));

        index_buffer_.update(meshData.indices, (sizeof(uint32_t) * meshData.indexCount));
    }
}
