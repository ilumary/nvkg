#include <nvkg/Renderer/Model/Model.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <cstring>

namespace std {
    template<>
    struct hash<nvkg::Vertex> {
        size_t operator()(const nvkg::Vertex &vertex) const {
            size_t seed = 0;
            nvkg::Utils::HashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        };
    };

    template<>
    struct hash<nvkg::Vertex2D> {
        size_t operator()(const nvkg::Vertex2D &vertex) const {
            size_t seed = 0;
            nvkg::Utils::HashCombine(seed, vertex.position, vertex.color);
            return seed;
        };
    };
}

namespace nvkg {
    Model::Model(const Mesh::MeshData& meshData) {
        mesh_.load_vertices(meshData);
    }

    Model::Model(const char* filePath) {
        LoadModelFromFile(filePath);
    }

    Model::Model() {}
    Model::~Model() {}

    void Model::LoadModelFromFile(const char* filePath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        NVKG_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath),
            warn + err);
        
        std::vector<Vertex> objVertices;
        std::vector<uint32_t> objIndices;
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(objVertices.size());
                    objVertices.push_back(vertex);
                }
                objIndices.push_back(uniqueVertices[vertex]);
            }
        }

        mesh_.load_vertices(
            {
                sizeof(Vertex),
                objVertices.data(), 
                static_cast<uint32_t>(objVertices.size()), 
                objIndices.data(), 
                static_cast<uint32_t>(objIndices.size())
            }
        );
    }

    void Model::update_mesh(const Mesh::MeshData& meshData) {
        mesh_.update_vertices(meshData);
    }

    void Model::set_mesh(const Mesh::MeshData& meshData) {
        mesh_.load_vertices(meshData);
    }

    void Model::bind(VkCommandBuffer commandBuffer, uint32_t bind_id) {
        mesh_.bind(commandBuffer, bind_id);
    }

    void Model::draw(VkCommandBuffer commandBuffer, uint32_t instance) {
        if (mesh_.has_index_buffer()) vkCmdDrawIndexed(commandBuffer, mesh_.get_index_count(), 1, 0, 0, instance);
        else vkCmdDraw(commandBuffer, mesh_.get_vertex_count(), 1, 0, instance);
    }
}
