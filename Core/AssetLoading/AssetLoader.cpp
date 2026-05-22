#include "AssetLoader.hpp"
#include <GL/glew.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

/* stb_image — header-only, inclua uma vez no projeto */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* ════════════════════════════════════════
   Utilitários
════════════════════════════════════════ */

std::string AssetLoader::extension_(const std::string& path) {
    auto pos = path.rfind('.');
    if (pos == std::string::npos) return "";
    std::string ext = path.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string AssetLoader::read_file_(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        printf("[AssetLoader] ERRO: nao abriu '%s'\n", path.c_str());
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

/* ════════════════════════════════════════
   Texturas
════════════════════════════════════════ */

Texture* AssetLoader::load_texture(const std::string& path) {

    /* cache hit */
    if (texture_cache_.count(path))
        return &texture_cache_[path];

    std::string ext = extension_(path);
    if (ext != "png" && ext != "jpg" && ext != "jpeg"
     && ext != "bmp" && ext != "tga") {
        printf("[AssetLoader] textura: formato '%s' nao suportado\n", ext.c_str());
        return nullptr;
    }

    Texture tex = load_png_jpg_(path);
    if (!tex.loaded) return nullptr;

    texture_cache_[path] = tex;
    printf("[AssetLoader] textura carregada: %s (%dx%d ch=%d)\n",
           path.c_str(), tex.width, tex.height, tex.channels);
    return &texture_cache_[path];
}

Texture AssetLoader::load_png_jpg_(const std::string& path) {
    Texture tex;
    tex.path = path;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(),
                                    &tex.width, &tex.height,
                                    &tex.channels, 0);
    if (!data) {
        printf("[AssetLoader] stbi erro: %s\n", stbi_failure_reason());
        return tex;
    }

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    GLenum fmt = tex.channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt,
                 tex.width, tex.height, 0, fmt,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    tex.loaded = true;
    return tex;
}

void AssetLoader::unload_texture(const std::string& path) {
    auto it = texture_cache_.find(path);
    if (it == texture_cache_.end()) return;
    glDeleteTextures(1, &it->second.id);
    texture_cache_.erase(it);
}

/* ════════════════════════════════════════
   Modelos 3D
════════════════════════════════════════ */

Model* AssetLoader::load_model(const std::string& path) {

    if (model_cache_.count(path))
        return &model_cache_[path];

    std::string ext = extension_(path);
    Model model;
    model.name = path.substr(path.rfind('/') + 1);
    model.path = path;

    bool ok = false;
    if      (ext == "obj")            ok = load_obj_(path, model);
    else if (ext == "gltf" || ext == "glb") ok = load_gltf_(path, model);
    else {
        printf("[AssetLoader] modelo: formato '%s' nao suportado\n", ext.c_str());
        return nullptr;
    }

    if (!ok) return nullptr;

    /* envia todas as submeshes para GPU */
    for (auto& mesh : model.meshes)
        upload_mesh_gpu_(mesh);

    model.loaded = true;
    model_cache_[path] = std::move(model);
    printf("[AssetLoader] modelo carregado: %s (%zu meshes)\n",
           path.c_str(), model_cache_[path].meshes.size());
    return &model_cache_[path];
}

/* ── .OBJ parser ── */
bool AssetLoader::load_obj_(const std::string& path, Model& out) {
    std::string src = read_file_(path);
    if (src.empty()) return false;

    SubMesh mesh;
    std::vector<float[3]> pos_list;
    std::vector<float[3]> norm_list;
    std::vector<float[2]> uv_list;

    std::istringstream ss(src);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ls(line);
        std::string token; ls >> token;

        if (token == "v") {
            float x,y,z; ls >> x >> y >> z;
            float p[3] = {x,y,z};
            pos_list.push_back({});
            pos_list.back()[0]=x; pos_list.back()[1]=y; pos_list.back()[2]=z;
        }
        else if (token == "vn") {
            float x,y,z; ls >> x >> y >> z;
            norm_list.push_back({});
            norm_list.back()[0]=x; norm_list.back()[1]=y; norm_list.back()[2]=z;
        }
        else if (token == "vt") {
            float u,v; ls >> u >> v;
            uv_list.push_back({});
            uv_list.back()[0]=u; uv_list.back()[1]=v;
        }
        else if (token == "usemtl") {
            ls >> mesh.material_name;
        }
        else if (token == "f") {
            /* suporta tri e quad, formato v/vt/vn */
            std::vector<Vertex> face_verts;
            std::string fv;
            while (ls >> fv) {
                Vertex vert = {};
                int pi=0, ti=0, ni=0;
                if (sscanf(fv.c_str(), "%d/%d/%d", &pi, &ti, &ni) == 3) {
                    if (pi > 0 && pi <= (int)pos_list.size()) {
                        vert.px=pos_list[pi-1][0];
                        vert.py=pos_list[pi-1][1];
                        vert.pz=pos_list[pi-1][2];
                    }
                    if (ti > 0 && ti <= (int)uv_list.size()) {
                        vert.u=uv_list[ti-1][0];
                        vert.v=uv_list[ti-1][1];
                    }
                    if (ni > 0 && ni <= (int)norm_list.size()) {
                        vert.nx=norm_list[ni-1][0];
                        vert.ny=norm_list[ni-1][1];
                        vert.nz=norm_list[ni-1][2];
                    }
                } else if (sscanf(fv.c_str(), "%d//%d", &pi, &ni) == 2) {
                    if (pi > 0 && pi <= (int)pos_list.size()) {
                        vert.px=pos_list[pi-1][0];
                        vert.py=pos_list[pi-1][1];
                        vert.pz=pos_list[pi-1][2];
                    }
                } else if (sscanf(fv.c_str(), "%d", &pi) == 1) {
                    if (pi > 0 && pi <= (int)pos_list.size()) {
                        vert.px=pos_list[pi-1][0];
                        vert.py=pos_list[pi-1][1];
                        vert.pz=pos_list[pi-1][2];
                    }
                }
                face_verts.push_back(vert);
            }

            /* triangula a face */
            for (int i = 1; i + 1 < (int)face_verts.size(); i++) {
                unsigned int base = (unsigned int)mesh.vertices.size();
                mesh.vertices.push_back(face_verts[0]);
                mesh.vertices.push_back(face_verts[i]);
                mesh.vertices.push_back(face_verts[i+1]);
                mesh.indices.push_back(base);
                mesh.indices.push_back(base+1);
                mesh.indices.push_back(base+2);
            }
        }
    }

    out.meshes.push_back(std::move(mesh));
    return true;
}

/* ── .GLTF / .GLB (estrutura básica via JSON) ── */
bool AssetLoader::load_gltf_(const std::string& path, Model& out) {
    printf("[AssetLoader] gltf: '%s' — suporte completo em breve\n", path.c_str());
    /* TODO: parsear JSON do gltf, carregar buffers binários */
    (void)out;
    return false;
}

/* ── Upload para GPU ── */
void AssetLoader::upload_mesh_gpu_(SubMesh& mesh) {
    if (mesh.vertices.empty()) return;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);

    /* posição */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    /* normal */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(12));
    glEnableVertexAttribArray(1);
    /* UV */
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(24));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    mesh.on_gpu = true;
}

void AssetLoader::unload_model(const std::string& path) {
    auto it = model_cache_.find(path);
    if (it == model_cache_.end()) return;
    for (auto& mesh : it->second.meshes) {
        if (mesh.on_gpu) {
            glDeleteVertexArrays(1, &mesh.vao);
            glDeleteBuffers(1, &mesh.vbo);
            glDeleteBuffers(1, &mesh.ebo);
        }
    }
    model_cache_.erase(it);
}

/* ════════════════════════════════════════
   JSON parser simples sem dependência
════════════════════════════════════════ */

std::string skip_ws_(const std::string& s, int& p) {
    while (p < (int)s.size() && isspace(s[p])) p++;
    return "";
}

JsonValue AssetLoader::load_json(const std::string& path) {
    std::string src = read_file_(path);
    int pos = 0;
    return parse_json_value_(src, pos);
}

JsonValue AssetLoader::parse_json_value_(const std::string& s, int& p) {
    skip_ws_(s, p);
    if (p >= (int)s.size()) return {};
    if (s[p] == '{') return parse_json_object_(s, p);
    if (s[p] == '[') return parse_json_array_(s, p);
    if (s[p] == '"') {
        JsonValue v; v.raw = parse_json_string_(s, p); return v;
    }
    /* número ou bool ou null */
    int start = p;
    while (p < (int)s.size() && s[p] != ',' && s[p] != '}' && s[p] != ']' && !isspace(s[p]))
        p++;
    JsonValue v; v.raw = s.substr(start, p - start); return v;
}

JsonValue AssetLoader::parse_json_object_(const std::string& s, int& p) {
    JsonValue obj;
    p++; // pula '{'
    while (p < (int)s.size()) {
        skip_ws_(s, p);
        if (s[p] == '}') { p++; break; }
        if (s[p] == ',') { p++; continue; }
        std::string key = parse_json_string_(s, p);
        skip_ws_(s, p);
        if (p < (int)s.size() && s[p] == ':') p++;
        obj.fields[key] = parse_json_value_(s, p);
    }
    return obj;
}

JsonValue AssetLoader::parse_json_array_(const std::string& s, int& p) {
    JsonValue arr;
    p++; // pula '['
    while (p < (int)s.size()) {
        skip_ws_(s, p);
        if (s[p] == ']') { p++; break; }
        if (s[p] == ',') { p++; continue; }
        arr.items.push_back(parse_json_value_(s, p));
    }
    return arr;
}

std::string AssetLoader::parse_json_string_(const std::string& s, int& p) {
    if (p < (int)s.size() && s[p] == '"') p++;
    int start = p;
    while (p < (int)s.size() && s[p] != '"') {
        if (s[p] == '\\') p++;
        p++;
    }
    std::string result = s.substr(start, p - start);
    if (p < (int)s.size()) p++;
    return result;
}

/* ════════════════════════════════════════
   Texto raw
════════════════════════════════════════ */

std::string AssetLoader::load_text(const std::string& path) {
    return read_file_(path);
}

/* ════════════════════════════════════════
   Cache
════════════════════════════════════════ */

void AssetLoader::clear_cache() {
    for (auto& [path, tex] : texture_cache_)
        glDeleteTextures(1, &tex.id);
    texture_cache_.clear();

    for (auto& [path, model] : model_cache_)
        for (auto& mesh : model.meshes)
            if (mesh.on_gpu) {
                glDeleteVertexArrays(1, &mesh.vao);
                glDeleteBuffers(1, &mesh.vbo);
                glDeleteBuffers(1, &mesh.ebo);
            }
    model_cache_.clear();
    printf("[AssetLoader] cache limpo\n");
}

bool AssetLoader::is_cached_texture(const std::string& path) const {
    return texture_cache_.count(path) > 0;
}

bool AssetLoader::is_cached_model(const std::string& path) const {
    return model_cache_.count(path) > 0;
}

void AssetLoader::print_cache() const {
    printf("[AssetLoader] cache — texturas: %zu  modelos: %zu\n",
           texture_cache_.size(), model_cache_.size());
    for (auto& [p, t] : texture_cache_)
        printf("  TEX  %s (%dx%d)\n", p.c_str(), t.width, t.height);
    for (auto& [p, m] : model_cache_)
        printf("  MDL  %s (%zu meshes)\n", p.c_str(), m.meshes.size());
}
