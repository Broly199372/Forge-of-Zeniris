#pragma once
#include <vector>
#include <cmath>

/* ─────────────────────────────────────────
   Vec3 — vetor 3D
───────────────────────────────────────── */
struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3  operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3  operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3  operator*(float s)       const { return {x*s,   y*s,   z*s  }; }
    Vec3& operator+=(const Vec3& o)      { x+=o.x; y+=o.y; z+=o.z; return *this; }

    float dot(const Vec3& o)  const { return x*o.x + y*o.y + z*o.z; }
    float length()            const { return sqrtf(x*x + y*y + z*z); }
    float length_sq()         const { return x*x + y*y + z*z; }

    Vec3 normalized() const {
        float l = length();
        return l > 1e-8f ? *this * (1.f / l) : Vec3{};
    }
};


/* ─────────────────────────────────────────
   Material — densidade e coeficientes
───────────────────────────────────────── */
struct Material {
    float density;           // kg/m³
    float restitution;       // 0 = mole,  1 = elástico puro
    float friction;          // 0 = gelo,  1 = borracha

    static Material Rock()   { return {2700.f, 0.2f, 0.6f}; }
    static Material Wood()   { return { 600.f, 0.3f, 0.4f}; }
    static Material Metal()  { return {7800.f, 0.4f, 0.5f}; }
    static Material Rubber() { return {1100.f, 0.8f, 0.8f}; }
    static Material Flesh()  { return {1010.f, 0.1f, 0.5f}; }
    static Material Ice()    { return { 917.f, 0.1f, 0.02f}; }
};


/* ─────────────────────────────────────────
   RigidBody — corpo físico no mundo
───────────────────────────────────────── */
struct RigidBody {
    int   id            = -1;
    Vec3  position      = {};
    Vec3  velocity      = {};
    Vec3  force         = {};       // forças acumuladas no frame
    float radius        = 0.5f;     // metros
    float mass          = 1.0f;     // kg  (calculado de density * volume)
    float gravity_scale = 1.0f;     // 0 = sem gravidade, 2 = gravidade dupla
    bool  is_static     = false;    // paredes, chão, objetos fixos
    bool  sleeping      = false;    // corpo parado — sem processar
    bool  use_gravity   = true;

    Material material   = Material::Rock();

    // Recalcula massa a partir de density e volume da esfera
    void recalculate_mass() {
        float volume = (4.f / 3.f) * 3.14159f * radius * radius * radius;
        mass = material.density * volume;
    }

    void apply_force(Vec3 f) { force += f; }

    float kinetic_energy() const {
        return 0.5f * mass * velocity.length_sq();
    }
};


/* ─────────────────────────────────────────
   ImpactEvent — dados de uma colisão
───────────────────────────────────────── */
struct ImpactEvent {
    int   body_a      = -1;
    int   body_b      = -1;    // -1 = chão
    Vec3  point       = {};
    Vec3  normal      = {};
    float impulse     = 0;
    float damage      = 0;     // 0..1  para usar no sistema de dano
};


/* ─────────────────────────────────────────
   PhysicsEngine — mundo físico completo
───────────────────────────────────────── */
class PhysicsEngine {
public:

    Vec3  gravity     = {0, -9.81f, 0};
    float air_density = 1.225f;       // kg/m³ — resistência do ar
    int   substeps    = 4;            // mais substeps = mais preciso

    std::vector<ImpactEvent> impacts; // impactos do último frame

    // Adiciona corpo ao mundo — retorna o ID
    int add(RigidBody body);

    // Busca corpo por ID
    RigidBody* get(int id);

    // Step principal — chame a cada frame com o delta time
    void update(float deltaTime);

    const std::vector<RigidBody>& bodies() const { return bodies_; }
          std::vector<RigidBody>& bodies()       { return bodies_; }

private:
    std::vector<RigidBody> bodies_;
    int next_id_ = 0;

    void apply_gravity_and_drag();
    void integrate(float dt);
    void resolve_floor();
    void resolve_sphere_sphere();
    void clear_forces();
};
