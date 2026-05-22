#pragma once
#include <vector>
#include <string>
#include <memory>
#include "../Physics/PhysicsEngine.hpp"

/* ─────────────────────────────────────────
   Entity — objeto do mundo
───────────────────────────────────────── */
struct Entity {
    int         id       = -1;
    std::string name     = "";
    bool        active   = true;

    int         body_id  = -1;  // ID no PhysicsEngine (-1 = sem física)
    std::string script   = "";  // caminho do .zn (vazio = sem script)

    Vec3        scale    = {1, 1, 1};
};


/* ─────────────────────────────────────────
   Scene — uma cena do jogo
───────────────────────────────────────── */
struct Scene {
    std::string            name;
    std::vector<Entity>    entities;
    PhysicsEngine          physics;

    Entity* get(int id) {
        for (auto& e : entities)
            if (e.id == id) return &e;
        return nullptr;
    }

    Entity* get(const std::string& name) {
        for (auto& e : entities)
            if (e.name == name) return &e;
        return nullptr;
    }
};


/* ─────────────────────────────────────────
   SceneManager — gerencia cenas e entidades
───────────────────────────────────────── */
class SceneManager {
public:

    // Cria uma nova cena e a torna ativa
    Scene* create(const std::string& name);

    // Troca a cena ativa
    bool   load(const std::string& name);

    // Cena ativa no momento
    Scene* current();

    // Adiciona entidade na cena ativa
    // body = nullptr significa entidade sem física (ex: câmera, luz)
    Entity* add_entity(const std::string& name,
                       Vec3 position      = {},
                       RigidBody* body    = nullptr,
                       const std::string& script = "");

    // Remove entidade por ID
    void remove_entity(int id);

    // Atualiza física e scripts da cena ativa
    void update(float delta);

    // Lista todas as cenas
    const std::vector<Scene>& scenes() const { return scenes_; }

private:
    std::vector<Scene> scenes_;
    int   current_index_ = -1;
    int   next_entity_id_ = 0;
};
