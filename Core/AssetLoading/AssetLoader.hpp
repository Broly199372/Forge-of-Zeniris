#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/* ─────────────────────────────────────────
   Texture — textura 2D na GPU
───────────────────────────────────────── */
struct Texture {
    unsigned int id       = 0;
    int          width    = 0;
    int          height   = 0;
    int          channels = 0;
    std::string  path     = "";
    bool         loaded   = false;
};


/* ─────────────────────────────────────────
   Vertex — vértice 3D completo
───────────────────────────────────────── */
struct Vertex {
    float px, py, pz;   // posição
    float nx, ny, nz;   // normal
    float u,  v;        // UV
};


/* ─────────────────────────────────────────
   SubMesh — parte de um modelo
───────────────────────────────────────── */
struct SubMesh {
    std::vector<Vertex>        vertices;
    std::vector<unsigned int>  indices;
    std::string                material_name;
    unsigned int               vao      = 0;
    unsigned int               vbo      = 0;
    unsigned int               ebo      = 0;
    bool                       on_gpu   = false;
};


/* ─────────────────────────────────────────
   Model — modelo 3D completo
───────────────────────────────────────── */
struct Model {
    std::string            name;
    std::string            path;
    std::vector<SubMesh>   meshes;
    bool                   loaded = false;
};


/* ─────────────────────────────────────────
   JsonValue — JSON simples sem dependência
───────────────────────────────────────── */
struct JsonValue {
    std::string                                    raw;
    std::unordered_map<std::string, JsonValue>     fields;
    std::vector<JsonValue>                         items;

    std::string str()         const { return raw; }
    float       number()      const { return std::stof(raw); }
    bool        boolean()     const { return raw == "true"; }
    bool        has(const std::string& k) const { return fields.count(k) > 0; }
    const JsonValue& operator[](const std::string& k) const { return fields.at(k); }
};


/* ─────────────────────────────────────────
   AssetLoader — carrega e cacheia assets
───────────────────────────────────────── */
class AssetLoader {
public:

    /* ── Texturas ── */
    /* suporta: .png .jpg .bmp .tga */
    Texture* load_texture(const std::string& path);
    void     unload_texture(const std::string& path);

    /* ── Modelos 3D ── */
    /* suporta: .obj .gltf .glb */
    Model*   load_model(const std::string& path);
    void     unload_model(const std::string& path);

    /* ── JSON ── */
    /* suporta: .json */
    JsonValue load_json(const std::string& path);

    /* ── Texto raw ── */
    /* suporta: .zn .zni .glsl .txt qualquer texto */
    std::string load_text(const std::string& path);

    /* ── Cache ── */
    void clear_cache();
    bool is_cached_texture(const std::string& path) const;
    bool is_cached_model(const std::string& path)   const;

    /* ── Info ── */
    void print_cache() const;

private:
    std::unordered_map<std::string, Texture> texture_cache_;
    std::unordered_map<std::string, Model>   model_cache_;

    /* loaders internos por formato */
    bool load_obj_(const std::string& path, Model& out);
    bool load_gltf_(const std::string& path, Model& out);

    Texture load_png_jpg_(const std::string& path);

    JsonValue parse_json_(const std::string& src, int& pos);
    JsonValue parse_json_value_(const std::string& src, int& pos);
    JsonValue parse_json_object_(const std::string& src, int& pos);
    JsonValue parse_json_array_(const std::string& src, int& pos);
    std::string parse_json_string_(const std::string& src, int& pos);

    void upload_mesh_gpu_(SubMesh& mesh);

    static std::string extension_(const std::string& path);
    static std::string read_file_(const std::string& path);
};
