#include "VulkanRenderer.hpp"
#include <stdio.h>

/*
 * VulkanRenderer — implementação progressiva
 *
 * A inicialização Vulkan tem muitos passos obrigatórios.
 * Cada função abaixo é um passo isolado — fácil de expandir.
 *
 * Dependência: vulkan, glfw3
 * Compilar com: -lvulkan -lglfw
 */

bool VulkanRenderer::init(int width, int height, const std::string& title) {
    width_  = width;
    height_ = height;

    if (!create_instance_(title))  return false;
    if (!create_device_())         return false;
    if (!create_swapchain_())      return false;
    if (!create_pipeline_())       return false;

    printf("[Vulkan] iniciado %dx%d\n", width, height);
    return true;
}

void VulkanRenderer::shutdown() {
    /* destrói objetos Vulkan na ordem inversa */
    printf("[Vulkan] encerrado\n");
}

/* ════════════════════════════════════════
   Passos de inicialização
════════════════════════════════════════ */

bool VulkanRenderer::create_instance_(const std::string& title) {
    printf("[Vulkan] criando instance: %s\n", title.c_str());
    /* VkApplicationInfo + VkInstanceCreateInfo aqui */
    return true;
}

bool VulkanRenderer::create_device_() {
    printf("[Vulkan] selecionando physical device\n");
    /* vkEnumeratePhysicalDevices → escolhe GPU aqui */
    return true;
}

bool VulkanRenderer::create_swapchain_() {
    printf("[Vulkan] criando swapchain %dx%d\n", width_, height_);
    /* VkSwapchainCreateInfoKHR aqui */
    return true;
}

bool VulkanRenderer::create_pipeline_() {
    printf("[Vulkan] criando pipeline\n");
    /* shaders SPIR-V + VkGraphicsPipelineCreateInfo aqui */
    return true;
}

/* ════════════════════════════════════════
   Frame
════════════════════════════════════════ */

void VulkanRenderer::begin_frame(const Camera& cam) {
    (void)cam;
    /* vkAcquireNextImageKHR + vkBeginCommandBuffer aqui */
}

void VulkanRenderer::end_frame() {
    /* vkEndCommandBuffer + vkQueueSubmit + vkQueuePresentKHR aqui */
}

bool VulkanRenderer::should_close() const {
    return false;
}

/* ════════════════════════════════════════
   Primitivas
════════════════════════════════════════ */

void VulkanRenderer::draw_sphere(Vec3 pos, float radius, Color col) {
    (void)pos; (void)radius; (void)col;
    /* push constants + draw call aqui */
}

void VulkanRenderer::draw_box(Vec3 pos, Vec3 he, Color col) {
    (void)pos; (void)he; (void)col;
}

void VulkanRenderer::draw_grid(int size, float spacing) {
    (void)size; (void)spacing;
}
