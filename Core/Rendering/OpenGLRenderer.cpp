#include "OpenGLRenderer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <stdio.h>

static GLFWwindow* window_ = nullptr;

/* ── Shaders básicos ── */
static const char* VERT_SRC = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uModel;
uniform mat4 uViewProj;
void main() {
    gl_Position = uViewProj * uModel * vec4(aPos, 1.0);
}
)";

static const char* FRAG_SRC = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
}
)";

/* ════════════════════════════════════════
   Init / Shutdown
════════════════════════════════════════ */

bool OpenGLRenderer::init(int width, int height, const std::string& title) {
    width_  = width;
    height_ = height;

    if (!glfwInit()) {
        printf("[OpenGL] ERRO: glfwInit falhou\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        printf("[OpenGL] ERRO: janela nao criada\n");
        return false;
    }

    glfwMakeContextCurrent(window_);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        printf("[OpenGL] ERRO: glewInit falhou\n");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    compile_shaders_();

    printf("[OpenGL] iniciado %dx%d\n", width, height);
    return true;
}

void OpenGLRenderer::shutdown() {
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
    printf("[OpenGL] encerrado\n");
}

/* ════════════════════════════════════════
   Frame
════════════════════════════════════════ */

void OpenGLRenderer::begin_frame(const Camera& cam) {
    glfwPollEvents();
    glClearColor(0.1f, 0.1f, 0.12f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_default_);
    set_view_proj_(cam);
}

void OpenGLRenderer::end_frame() {
    glfwSwapBuffers(window_);
}

bool OpenGLRenderer::should_close() const {
    return window_ && glfwWindowShouldClose(window_);
}

void OpenGLRenderer::set_wireframe(bool on) {
    glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

/* ════════════════════════════════════════
   Primitivas
════════════════════════════════════════ */

void OpenGLRenderer::draw_sphere(Vec3 pos, float radius, Color col) {
    /* gera esfera por latitude/longitude */
    const int stacks = 12, slices = 12;
    float verts[stacks * slices * 3 * 6];
    int   vi = 0;

    for (int i = 0; i < stacks; i++) {
        float phi0 = 3.14159f * (-0.5f + (float)i       / stacks);
        float phi1 = 3.14159f * (-0.5f + (float)(i + 1) / stacks);
        for (int j = 0; j < slices; j++) {
            float th0 = 2*3.14159f * (float)j       / slices;
            float th1 = 2*3.14159f * (float)(j + 1) / slices;
            auto push = [&](float ph, float th) {
                verts[vi++] = pos.x + radius * cosf(ph) * cosf(th);
                verts[vi++] = pos.y + radius * sinf(ph);
                verts[vi++] = pos.z + radius * cosf(ph) * sinf(th);
            };
            push(phi0,th0); push(phi1,th0); push(phi1,th1);
            push(phi0,th0); push(phi1,th1); push(phi0,th1);
        }
    }

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    glGenBuffers(1, &vbo);      glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vi * sizeof(float), verts, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);
    glEnableVertexAttribArray(0);

    /* model = identidade (posição já embutida nos verts) */
    float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    glUniformMatrix4fv(glGetUniformLocation(shader_default_,"uModel"),
                       1, GL_FALSE, identity);
    glUniform4f(glGetUniformLocation(shader_default_,"uColor"),
                col.r, col.g, col.b, col.a);

    glDrawArrays(GL_TRIANGLES, 0, vi / 3);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void OpenGLRenderer::draw_grid(int size, float spacing) {
    glUseProgram(shader_default_);
    float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    glUniformMatrix4fv(glGetUniformLocation(shader_default_,"uModel"),
                       1, GL_FALSE, identity);
    glUniform4f(glGetUniformLocation(shader_default_,"uColor"),
                0.3f, 0.3f, 0.3f, 1.f);

    float half = size * spacing * 0.5f;
    for (int i = -size/2; i <= size/2; i++) {
        float fi = i * spacing;
        float lines[] = {
            fi, 0, -half,   fi, 0, half,
            -half, 0, fi,   half, 0, fi
        };
        unsigned int vao, vbo;
        glGenVertexArrays(1,&vao); glBindVertexArray(vao);
        glGenBuffers(1,&vbo);      glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(lines),lines,GL_STREAM_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,12,(void*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES,0,4);
        glDeleteBuffers(1,&vbo);
        glDeleteVertexArrays(1,&vao);
    }
}

/* ════════════════════════════════════════
   Shaders
════════════════════════════════════════ */

void OpenGLRenderer::compile_shaders_() {
    auto compile = [](const char* src, GLenum type) -> unsigned int {
        unsigned int s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512]; glGetShaderInfoLog(s,512,nullptr,log);
            printf("[OpenGL] shader erro: %s\n", log);
        }
        return s;
    };

    unsigned int vs = compile(VERT_SRC, GL_VERTEX_SHADER);
    unsigned int fs = compile(FRAG_SRC, GL_FRAGMENT_SHADER);
    shader_default_ = glCreateProgram();
    glAttachShader(shader_default_, vs);
    glAttachShader(shader_default_, fs);
    glLinkProgram(shader_default_);
    glDeleteShader(vs);
    glDeleteShader(fs);
    printf("[OpenGL] shaders compilados\n");
}

/* ════════════════════════════════════════
   View / Projection  (matriz manual)
════════════════════════════════════════ */

void OpenGLRenderer::set_view_proj_(const Camera& cam) {
    /* lookAt manual */
    Vec3 f = (cam.target - cam.position).normalized();
    Vec3 r = f.cross(cam.up).normalized();
    Vec3 u = r.cross(f);

    float view[16] = {
         r.x,  u.x, -f.x, 0,
         r.y,  u.y, -f.y, 0,
         r.z,  u.z, -f.z, 0,
        -r.dot(cam.position), -u.dot(cam.position), f.dot(cam.position), 1
    };

    /* perspectiva manual */
    float aspect = (float)width_ / (float)height_;
    float fov_r  = cam.fov * 3.14159f / 180.f;
    float t      = tanf(fov_r * 0.5f);
    float near   = cam.near_clip, far = cam.far_clip;

    float proj[16] = {
        1/(aspect*t), 0,    0,                          0,
        0,            1/t,  0,                          0,
        0,            0,   -(far+near)/(far-near),     -1,
        0,            0,   -(2*far*near)/(far-near),    0
    };

    /* ViewProj = Proj * View */
    float vp[16] = {};
    for (int row=0;row<4;row++)
        for (int col=0;col<4;col++)
            for (int k=0;k<4;k++)
                vp[row*4+col] += proj[row*4+k] * view[k*4+col];

    glUniformMatrix4fv(glGetUniformLocation(shader_default_,"uViewProj"),
                       1, GL_FALSE, vp);
}
