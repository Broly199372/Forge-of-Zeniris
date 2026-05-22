#include "PhysicsEngine.hpp"
#include <algorithm>

/* ─── constantes ─── */
static constexpr float DRAG_COEFF   = 0.47f;   // esfera
static constexpr float SLEEP_SPEED  = 0.01f;   // m/s — abaixo disso dorme

/* ════════════════════════════════════════
   API pública
════════════════════════════════════════ */

int PhysicsEngine::add(RigidBody body) {
    body.id = next_id_++;
    body.recalculate_mass();
    bodies_.push_back(body);
    return body.id;
}

RigidBody* PhysicsEngine::get(int id) {
    for (auto& b : bodies_)
        if (b.id == id) return &b;
    return nullptr;
}

void PhysicsEngine::update(float deltaTime) {
    impacts.clear();

    float dt = deltaTime / (float)substeps;

    for (int s = 0; s < substeps; s++) {
        apply_gravity_and_drag();
        integrate(dt);
        resolve_floor();
        resolve_sphere_sphere();
        clear_forces();
    }
}

/* ════════════════════════════════════════
   Gravidade e arrasto aerodinâmico
════════════════════════════════════════ */

void PhysicsEngine::apply_gravity_and_drag() {
    for (auto& b : bodies_) {
        if (b.is_static || b.sleeping) continue;

        /* Gravidade:  F = m * g * scale */
        if (b.use_gravity)
            b.force += gravity * (b.mass * b.gravity_scale);

        /* Arrasto:  F = -½ · ρ · Cd · A · v² · v̂ */
        float speed = b.velocity.length();
        if (speed > 1e-4f) {
            float area = 3.14159f * b.radius * b.radius;
            float drag = 0.5f * air_density * DRAG_COEFF * area * speed * speed;
            b.force += b.velocity.normalized() * (-drag);
        }
    }
}

/* ════════════════════════════════════════
   Integração  (Euler semi-implícito)
════════════════════════════════════════ */

void PhysicsEngine::integrate(float dt) {
    for (auto& b : bodies_) {
        if (b.is_static || b.sleeping) continue;

        /* a = F / m */
        float inv_mass = 1.f / b.mass;
        b.velocity += b.force * (inv_mass * dt);

        /* amortecimento mínimo do ar */
        b.velocity = b.velocity * 0.999f;

        /* atualiza posição */
        b.position += b.velocity * dt;

        /* verifica se pode dormir */
        b.sleeping = b.velocity.length_sq() < (SLEEP_SPEED * SLEEP_SPEED);
    }
}

/* ════════════════════════════════════════
   Colisão com o chão  (Y = 0)
════════════════════════════════════════ */

void PhysicsEngine::resolve_floor() {
    for (auto& b : bodies_) {
        if (b.is_static) continue;

        float penetration = b.radius - b.position.y;
        if (penetration <= 0) continue;

        /* corrige posição */
        b.position.y = b.radius;

        float vn = b.velocity.y;
        if (vn >= 0) continue; /* já subindo */

        /* impulso normal:  j = -(1+e) · vn · m */
        float j = -(1.f + b.material.restitution) * vn * b.mass;
        b.velocity.y += j / b.mass;

        /* fricção lateral */
        b.velocity.x *= (1.f - b.material.friction * 0.1f);
        b.velocity.z *= (1.f - b.material.friction * 0.1f);

        b.sleeping = false;

        /* registra impacto */
        ImpactEvent ev;
        ev.body_a  = b.id;
        ev.body_b  = -1;   // chão
        ev.normal  = {0, 1, 0};
        ev.point   = {b.position.x, 0, b.position.z};
        ev.impulse = std::abs(j);
        ev.damage  = std::min(1.f, ev.impulse / (b.mass * 10.f));
        impacts.push_back(ev);
    }
}

/* ════════════════════════════════════════
   Colisão esfera × esfera
════════════════════════════════════════ */

void PhysicsEngine::resolve_sphere_sphere() {
    for (int i = 0; i < (int)bodies_.size(); i++) {
        for (int j = i + 1; j < (int)bodies_.size(); j++) {

            auto& a = bodies_[i];
            auto& b = bodies_[j];
            if (a.is_static && b.is_static) continue;

            Vec3  d    = b.position - a.position;
            float dist = d.length();
            float rsum = a.radius + b.radius;

            if (dist >= rsum || dist < 1e-6f) continue;

            Vec3  n   = d.normalized();
            float pen = rsum - dist;

            float inv_a = a.is_static ? 0.f : 1.f / a.mass;
            float inv_b = b.is_static ? 0.f : 1.f / b.mass;
            float inv_total = inv_a + inv_b;
            if (inv_total < 1e-8f) continue;

            /* ── separa os corpos proporcionalmente à massa ── */
            if (!a.is_static) a.position += n * (-pen * inv_a / inv_total);
            if (!b.is_static) b.position += n * ( pen * inv_b / inv_total);

            /* ── velocidade relativa na normal ── */
            Vec3  rv = b.velocity - a.velocity;
            float vn = rv.dot(n);
            if (vn > 0) continue; /* já se afastando */

            /* ── impulso de colisão ── */
            float e = std::min(a.material.restitution, b.material.restitution);
            float j_imp = -(1.f + e) * vn / inv_total;

            if (!a.is_static) a.velocity += n * (-j_imp * inv_a);
            if (!b.is_static) b.velocity += n * ( j_imp * inv_b);

            a.sleeping = b.sleeping = false;

            /* ── registra impacto ── */
            ImpactEvent ev;
            ev.body_a  = a.id;
            ev.body_b  = b.id;
            ev.normal  = n;
            ev.point   = a.position + n * a.radius;
            ev.impulse = std::abs(j_imp);
            ev.damage  = std::min(1.f, ev.impulse / ((a.mass + b.mass) * 5.f));
            impacts.push_back(ev);
        }
    }
}

/* ════════════════════════════════════════
   Limpa forças acumuladas
════════════════════════════════════════ */

void PhysicsEngine::clear_forces() {
    for (auto& b : bodies_)
        b.force = {};
}
