#ifndef SDF_TEXT_HPP
#define SDF_TEXT_HPP

#include <nvkg/Renderer/Model/Model.hpp>

#include <map>
#include <array>
#include <sstream>
#include <string>

namespace nvkg {

    struct sdf_text {
        public:

            static std::unique_ptr<nvkg::Model> generate_text(std::string text);

            static void update_model_mesh(std::string text, std::unique_ptr<nvkg::Model>& model, uint32_t start_index = 0);

            static void generate_text_old(std::string text, std::unique_ptr<nvkg::Model>& model);

            static const material_handle sdf_material();

        private:

            struct bmchar {
                uint32_t x, y;
                uint32_t width;
                uint32_t height;
                int32_t xoffset;
                int32_t yoffset;
                int32_t xadvance;
                uint32_t page;
            };

            struct vi_2d {
                std::vector<Vertex2D> vertices;
                std::vector<uint32_t> indices; 
            };

            struct vi_3d { //TODO
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices; 
            };

            static void generate_mesh_from_char(char c, vi_2d& mesh);

            static std::array<bmchar, 255> font_chars_;

            static std::map<char, vi_2d> chars_;
    };

}

#endif