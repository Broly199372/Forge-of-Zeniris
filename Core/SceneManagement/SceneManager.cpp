#include "SceneManager.hpp"
#include <stdio.h>

/* ════════════════════════════════════════
   Cenas
════════════════════════════════════════ */

Scene* SceneManager::create(const std::string& name) {
    Scene s;
    s.name = name;
    scenes_.push_back(std::move(s));
    current_index_ = (int)scenes_.size() - 1;
    printf("[Scene] '%s' criada\n", name.c_str());
    return &scenes_[current_index_];
}

bool SceneManager::load(const std::string& name) {
    for (int i = 0; i < (int)scenes_.size(); i++) {
        if (scenes_[i].name == name) {
            current_index_ = i;
            printf("[Scene] '%s' carregada\n", name.c_str());
            return true;
        }
    }
    printf("[Scene] ERRO: '%s' nao encontrada\n", name.c_str());
    return false;
}

Scene* SceneManager::current() {
    if (current_index_ < 0) return nullptr;
    return &scenes_[current_index_];
}

/* ════════════════════════════════════════
   Entidades
════════════════════════════════════════ */

Entity* SceneManager::add_entity(const std::string& name,
                                  Vec3 position,
                                  RigidBody* body,
                                  const std::string& script) {
    Scene* s = current();
    if (!s) {
        printf("[Scene] ERRO: nenhuma cena ativa\n");
        return nullptr;
    }

    Entity e;
    e.id     = next_entity_id_++;
    e.name   = name;
    e.script = script;

    /* se tem corpo físico, adiciona no PhysicsEngine da cena */
    if (body) {
        body->position = position;
        e.body_id = s->physics.add(*body);
    }

    s->entities.push_back(e);

    printf("[Scene] entidade '%s' adicionada (id=%d body=%d)\n",
           name.c_str(), e.id, e.body_id);

    return &s->entities.back();
}

void SceneManager::remove_entity(int id) {
    Scene* s = current();
    if (!s) return;
    auto& ev = s->entities;
    ev.erase(std::remove_if(ev.begin(), ev.end(),
        [id](const Entity& e){ return e.id == id; }), ev.end());
}

/* ════════════════════════════════════════
   Update
════════════════════════════════════════ */

void SceneManager::update(float delta) {
    Scene* s = current();
    if (!s) return;

    /* atualiza física */
    s->physics.update(delta);

    /* mostra impactos no console (depois conecta com Zeniris) */
    for (auto& impact : s->physics.impacts) {
        if (impact.damage > 0.3f)
            printf("[Physics] impacto forte! dano=%.0f%% impulso=%.1f\n",
                   impact.damage * 100.f, impact.impulse);
    }
}
