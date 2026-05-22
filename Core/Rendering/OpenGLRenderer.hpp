#pragma once
#include <string>
#include "../Physics/PhysicsEngine.hpp"

/* ─────────────────────────────────────────
   Color
───────────────────────────────────────── */
struct Color {
    float r=1, g=1, b=1, a=1;

    static Color White()  { return {1,1,1,1}; }
    static Color Black()  { return {0,0,0,1}; }
    static Color Red()    { return {1,0,0,1}; }
    static Color Green()  { return {0,1,0,1}; }
    static Color Blue()   { return {0,0,1,1}; }
};

/* ─────────────────────────────────────────
   Camera 3D
───────────────────────────────────────── */
struct Camera {
    Vec3  position  = {0, 5, 10};
    Vec3  target    = {0, 0,  0};
    Vec3  up        = {0, 1,  0};
    float fov       = 60.f;
    float near_clip = 0.1f;
    float far_clip  = 1000.f;
};

/* ─────────────────────────────────────────
   Mesh — geometria 3D
───────────────────────────────────────── */
struct Mesh {
    unsigned int vao = 0;   // Vertex Array Object
    unsigned int vbo = 0;   // Vertex Buffer
    unsigned int ebo = 0;   // Index Buffer
    int index_count  = 0;
};

/* ─────────────────────────────────────────
   OpenGLRenderer
───────────────────────────────────────── */
class OpenGLRenderer {
public:

    bool init(int width, int height, const std::string& title);
    void shutdown();

    void begin_frame(const Camera& cam);
    void end_frame();

    /* primitivas */
    void draw_sphere(Vec3 position, float radius, Color color);
    void draw_box(Vec3 position, Vec3 half_extents, Color color);
    void draw_line(Vec3 from, Vec3 to, Color color);
    void draw_grid(int size, float spacing);

    /* mesh customizada */
    Mesh create_mesh(float* vertices, int vert_count,
                     unsigned int* indices, int idx_count);
    void draw_mesh(const Mesh& mesh, Vec3 position, Vec3 scale, Color color);
    void destroy_mesh(Mesh& mesh);

    bool should_close() const;
    void set_wireframe(bool on);

    int width()  const { return width_;  }
    int height() const { return height_; }

private:
    int          width_  = 0;
    int          height_ = 0;
    unsigned int shader_default_ = 0;

    void compile_shaders_();
    void set_transform_(Vec3 pos, Vec3 scale);
    void set_view_proj_(const Camera& cam);
};
