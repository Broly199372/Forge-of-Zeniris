#pragma once
#include <string>
#include "SceneManagement/SceneManager.hpp"
#include "Input/InputManager.hpp"
#include "Rendering/OpenGLRenderer.hpp"
#include "Rendering/VulkanRenderer.hpp"

enum class RendererBackend { OpenGL, Vulkan };

/* ─────────────────────────────────────────
   GameLoop — coração da engine
   Une física, input, cena e renderer
───────────────────────────────────────── */
class GameLoop {
public:

    SceneManager  scene;
    InputManager  input;
    Camera        camera;

    bool init(const std::string& title,
              int width, int height,
              RendererBackend backend = RendererBackend::OpenGL);

    void run();
    void stop();
    void shutdown();

    /* qual backend está ativo */
    RendererBackend backend() const { return backend_; }

private:
    RendererBackend  backend_  = RendererBackend::OpenGL;
    OpenGLRenderer   opengl_;
    VulkanRenderer   vulkan_;

    bool running_    = false;
    float target_fps_ = 60.f;

    void tick_(float delta);
    float get_time_() const;
};
