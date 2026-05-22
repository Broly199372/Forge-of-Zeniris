#pragma once
#include <string>
#include <vector>
#include "../Physics/PhysicsEngine.hpp"
#include "OpenGLRenderer.hpp"  // reusa Color e Camera

/* ─────────────────────────────────────────
   VulkanRenderer
   Renderização de alta performance
───────────────────────────────────────── */
class VulkanRenderer {
public:

    bool init(int width, int height, const std::string& title);
    void shutdown();

    void begin_frame(const Camera& cam);
    void end_frame();

    void draw_sphere(Vec3 position, float radius, Color color);
    void draw_box(Vec3 position, Vec3 half_extents, Color color);
    void draw_grid(int size, float spacing);

    bool should_close() const;

    int width()  const { return width_;  }
    int height() const { return height_; }

private:
    int width_  = 0;
    int height_ = 0;

    /* handles Vulkan */
    void* instance_        = nullptr;
    void* device_          = nullptr;
    void* physical_device_ = nullptr;
    void* surface_         = nullptr;
    void* swapchain_       = nullptr;
    void* render_pass_     = nullptr;
    void* pipeline_        = nullptr;
    void* cmd_pool_        = nullptr;

    bool create_instance_(const std::string& title);
    bool create_device_();
    bool create_swapchain_();
    bool create_pipeline_();
};
