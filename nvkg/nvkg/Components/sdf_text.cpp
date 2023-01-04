#include <nvkg/Components/sdf_text.hpp>

namespace nvkg {

    std::array<sdf_text::bmchar, 255> sdf_text::font_chars_ = [] {
        std::array<sdf_text::bmchar, 255> chars_;

        auto next_value_pair = [](std::stringstream *stream) -> int32_t {
            std::string pair;
            *stream >> pair;
            uint32_t spos = pair.find("=");
            std::string value = pair.substr(spos + 1);
            int32_t val = std::stoi(value);
            return val;
        };

        std::string fileName = "../assets/fonts/font.fnt";

        std::filebuf fileBuffer;
        fileBuffer.open(fileName, std::ios::in);
        std::istream istream(&fileBuffer);

        assert(istream.good());

        while (!istream.eof()) {
            std::string line;
            std::stringstream lineStream;
            std::getline(istream, line);
            lineStream << line;

            std::string info;
            lineStream >> info;

            if (info == "char") {
                // char id
                uint32_t charid = next_value_pair(&lineStream);
                // Char properties
                chars_[charid].x = next_value_pair(&lineStream);
                chars_[charid].y = next_value_pair(&lineStream);
                chars_[charid].width = next_value_pair(&lineStream);
                chars_[charid].height = next_value_pair(&lineStream);
                chars_[charid].xoffset = next_value_pair(&lineStream);
                chars_[charid].yoffset = next_value_pair(&lineStream);
                chars_[charid].xadvance = next_value_pair(&lineStream);
                chars_[charid].page = next_value_pair(&lineStream);
            }
        }

        return chars_;
    }();

    void sdf_text::generate_mesh_from_char(char c, vi_2d& mesh) {
        std::vector<nvkg::Vertex2D> vertices;
        std::vector<uint32_t> indices;
        uint32_t indexOffset = 0, indexCount = 0;

        float w = 512; //textures.fontSDF.width; TODO

        float posx = 0.0f;
        float posy = 0.0f;

        bmchar *charInfo = &sdf_text::font_chars_[(int)c];

        if (charInfo->width == 0)
            charInfo->width = 36;

        float charw = ((float)(charInfo->width) / 36.0f);
        float dimx = 1.0f * charw;
        float charh = ((float)(charInfo->height) / 36.0f);
        float dimy = 1.0f * charh;

        float us = charInfo->x / w;
        float ue = (charInfo->x + charInfo->width) / w;
        float ts = charInfo->y / w;
        float te = (charInfo->y + charInfo->height) / w;

        float xo = charInfo->xoffset / 36.0f;
        float yo = charInfo->yoffset / 36.0f;

        posy = yo;

        vertices.push_back({ { posx + dimx + xo,  posy + dimy}, { ue, te }, {1.f, 0.f, 0.f, 1.f} });
        vertices.push_back({ { posx + xo,         posy + dimy}, { us, te }, {1.f, 0.f, 0.f, 1.f} });
        vertices.push_back({ { posx + xo,         posy,      }, { us, ts }, {1.f, 0.f, 0.f, 1.f} });
        vertices.push_back({ { posx + dimx + xo,  posy,      }, { ue, ts }, {1.f, 0.f, 0.f, 1.f} });

        std::array<uint32_t, 6> letterIndices = { 0,1,2, 2,3,0 };
        for (auto& index : letterIndices) {
            indices.push_back(indexOffset + index);
        }
        indexOffset += 4;

        float advance = ((float)(charInfo->xadvance) / 46.0f);
        posx += advance;

        indexCount = indices.size();

        mesh.indices = std::move(indices);
        mesh.vertices = std::move(vertices);
    }

    std::map<char, sdf_text::vi_2d> sdf_text::chars_ = []{
        std::map<char, vi_2d> tmp_chars;
        
        std::string alphanum = "abcdefghijklmnopqrstuvwqxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        std::string special_chars = "(){}[]|$@!?;/\\%&<>:+^='\"`*~ ,.";

        for(int i = 0; i < alphanum.size(); ++i) {
            generate_mesh_from_char(alphanum[i], tmp_chars[alphanum[i]]);
        }

        for(int i = 0; i < special_chars.size(); ++i) {
            generate_mesh_from_char(special_chars[i], tmp_chars[special_chars[i]]);
        }
        
        return tmp_chars;
    }();

    std::unique_ptr<Model> sdf_text::generate_text(std::string text) {
        std::vector<Vertex2D> vertices;
        std::vector<uint32_t> indices;

        uint32_t indice_offset = 0;
        float previous_stride_x = 0;

        for (uint32_t i = 0; i < text.size(); i++) {
            vi_2d* mesh = &chars_[text[i]];

            std::vector<uint32_t> tmp_i = mesh->indices;

            for(auto& i : tmp_i) {
                i += indice_offset;
            }

            indice_offset += 4;

            indices.insert(indices.end(), tmp_i.begin(), tmp_i.end());
            
            std::vector<Vertex2D> tmp_v = mesh->vertices;

            for(auto& v : tmp_v) {
                v.position.x += previous_stride_x;
            }

            previous_stride_x += (float)sdf_text::font_chars_[(int)text[i]].xadvance / 46.f;

            vertices.insert(vertices.end(), tmp_v.begin(), tmp_v.end());
        }

        return std::make_unique<Model>( Mesh::MeshData {
            sizeof(Vertex2D),
            vertices.data(),
            static_cast<uint32_t>(vertices.size()),
            indices.data(),
            static_cast<uint32_t>(indices.size()),
        });
    }

    void sdf_text::update_model_mesh(std::string text, std::unique_ptr<nvkg::Model>& model, uint32_t start_index) {
        std::vector<Vertex2D> vertices;
        std::vector<uint32_t> indices;

        uint32_t indice_offset = 0;
        float previous_stride_x = 0;

        for (uint32_t i = 0; i < text.size(); i++) {
            vi_2d* mesh = &chars_[text[i]];

            std::vector<uint32_t> tmp_i = mesh->indices;

            for(auto& i : tmp_i) {
                i += indice_offset;
            }

            indice_offset += 4;

            indices.insert(indices.end(), tmp_i.begin(), tmp_i.end());
            
            std::vector<Vertex2D> tmp_v = mesh->vertices;

            for(auto& v : tmp_v) {
                v.position.x += previous_stride_x;
            }

            previous_stride_x += (float)sdf_text::font_chars_[(int)text[i]].xadvance / 46.f;

            vertices.insert(vertices.end(), tmp_v.begin(), tmp_v.end());
        }

        model->update_mesh({
            sizeof(Vertex2D),
            vertices.data(),
            static_cast<uint32_t>(vertices.size()),
            indices.data(),
            static_cast<uint32_t>(indices.size())
        });
    }

    void sdf_text::generate_text_old(std::string text, std::unique_ptr<nvkg::Model>& model) {
        std::vector<nvkg::Vertex2D> vertices;
        std::vector<uint32_t> indices;
        uint32_t indexOffset = 0, indexCount = 0;

        float w = 512; //textures.fontSDF.width;

        float posx = 0.0f;
        float posy = 0.0f;

        for (uint32_t i = 0; i < text.size(); i++) {
            bmchar *charInfo = &sdf_text::font_chars_[(int)text[i]];

            if (charInfo->width == 0)
                charInfo->width = 36;

            float charw = ((float)(charInfo->width) / 36.0f);
            float dimx = 1.0f * charw;
            float charh = ((float)(charInfo->height) / 36.0f);
            float dimy = 1.0f * charh;

            float us = charInfo->x / w;
            float ue = (charInfo->x + charInfo->width) / w;
            float ts = charInfo->y / w;
            float te = (charInfo->y + charInfo->height) / w;

            float xo = charInfo->xoffset / 36.0f;
            float yo = charInfo->yoffset / 36.0f;

            posy = yo;

            vertices.push_back({ { posx + dimx + xo,  posy + dimy}, { ue, te }, {1.f, 0.f, 0.f, 1.f} });
            vertices.push_back({ { posx + xo,         posy + dimy}, { us, te }, {1.f, 0.f, 0.f, 1.f} });
            vertices.push_back({ { posx + xo,         posy,      }, { us, ts }, {1.f, 0.f, 0.f, 1.f} });
            vertices.push_back({ { posx + dimx + xo,  posy,      }, { ue, ts }, {1.f, 0.f, 0.f, 1.f} });

            std::array<uint32_t, 6> letterIndices = { 0,1,2, 2,3,0 };
            for (auto& index : letterIndices) {
                indices.push_back(indexOffset + index);
            }
            indexOffset += 4;

            float advance = ((float)(charInfo->xadvance) / 46.0f);
            posx += advance;
        }

        indexCount = indices.size();

        model->set_mesh({
            sizeof(Vertex2D),
            vertices.data(),
            static_cast<uint32_t>(vertices.size()),
            indices.data(),
            static_cast<uint32_t>(indices.size()),
        });
    }

}