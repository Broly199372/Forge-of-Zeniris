#include "GameLoop.hpp"
#include <GLFW/glfw3.h>
#include <stdio.h>

/* ════════════════════════════════════════
   Init
════════════════════════════════════════ */

bool GameLoop::init(const std::string& title,
                    int width, int height,
                    RendererBackend backend) {
    backend_ = backend;

    bool ok = false;

    if (backend_ == RendererBackend::OpenGL)
        ok = opengl_.init(width, height, title);
    else
        ok = vulkan_.init(width, height, title);

    if (!ok) {
        printf("[GameLoop] ERRO: renderer nao iniciou\n");
        return false;
    }

    printf("[GameLoop] iniciado — backend: %s\n",
           backend_ == RendererBackend::OpenGL ? "OpenGL" : "Vulkan");
    return true;
}

/* ════════════════════════════════════════
   Loop principal
════════════════════════════════════════ */

void GameLoop::run() {
    running_ = true;

    float prev_time = get_time_();
    float lag       = 0.f;
    float fixed_dt  = 1.f / target_fps_;

    printf("[GameLoop] rodando\n");

    while (running_) {

        /* verifica se janela fechou */
        bool closed = backend_ == RendererBackend::OpenGL
                    ? opengl_.should_close()
                    : vulkan_.should_close();
        if (closed) break;

        /* calcula delta */
        float now   = get_time_();
        float delta = now - prev_time;
        prev_time   = now;
        if (delta > 0.1f) delta = 0.1f; // evita espiral da morte

        lag += delta;

        /* input */
        input.begin_frame();

        /* física em passo fixo */
        while (lag >= fixed_dt) {
            scene.update(fixed_dt);
            lag -= fixed_dt;
        }

        /* render */
        tick_(delta);
    }

    printf("[GameLoop] encerrando\n");
}

void GameLoop::stop() {
    running_ = false;
}

void GameLoop::shutdown() {
    if (backend_ == RendererBackend::OpenGL)
        opengl_.shutdown();
    else
        vulkan_.shutdown();
}

/* ════════════════════════════════════════
   Tick — render do frame
════════════════════════════════════════ */

void GameLoop::tick_(float delta) {
    (void)delta;

    if (backend_ == RendererBackend::OpenGL) {

        opengl_.begin_frame(camera);
        opengl_.draw_grid(20, 1.f);

        /* desenha todos os corpos físicos da cena ativa */
        if (auto* s = scene.current()) {
            for (auto& body : s->physics.bodies()) {
                if (!body.active) continue;
                opengl_.draw_sphere(body.position, body.radius,
                                    body.is_static
                                    ? Color::White()
                                    : Color::Green());
            }

            /* debug — impactos do frame */
            for (auto& impact : s->physics.impacts) {
                opengl_.draw_sphere(impact.point, 0.05f, Color::Red());
            }
        }

        opengl_.end_frame();

    } else {

        vulkan_.begin_frame(camera);

        if (auto* s = scene.current()) {
            for (auto& body : s->physics.bodies()) {
                if (!body.active) continue;
                vulkan_.draw_sphere(body.position, body.radius, Color::Green());
            }
        }

        vulkan_.end_frame();
    }
}

/* ════════════════════════════════════════
   Tempo
════════════════════════════════════════ */

float GameLoop::get_time_() const {
    return (float)glfwGetTime();
}
