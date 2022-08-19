#pragma once

#include <nvkg/Renderer/Buffer/Buffer.hpp>
#include <nvkg/Renderer/Utils/Hash.hpp>
#include <nvkg/Renderer/Mesh/Mesh.hpp>
#include <nvkg/Renderer/Material/Material.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

namespace nvkg {
    
    class Model {
        public:

            struct Transform {
                glm::mat4 transform;
                glm::mat4 normalMatrix;
            };

            // Placeholder - in case we need to add more unique 2D data
            struct Transform2D {
                glm::mat4 transform;
            };

            Model(const Mesh::MeshData& meshData);
            Model(const char* filePath);
            Model();
            ~Model();

            void DestroyModel();

            Model(const Model&) = delete;
            Model& operator=(const Model&) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer, uint32_t instance = 0);

            NVKGMaterial* get_material() { return material; }
            void update_mesh(const Mesh::MeshData& meshData);
            void set_mesh(const Mesh::MeshData& meshData);
            void set_material(NVKGMaterial* newMaterial) { material = newMaterial; }

            bool IsIndexed() { return modelMesh.HasIndexBuffer(); }

        private:

            void LoadModelFromFile(const char* filePath);

            Mesh modelMesh;
            NVKGMaterial* material{nullptr};
    };
}
