#pragma once
#include <string>
#include <unordered_map>
#include "../Physics/PhysicsEngine.hpp"

/* ─────────────────────────────────────────
   Sound — som carregado na memória
───────────────────────────────────────── */
struct Sound {
    unsigned int buffer = 0;   // handle OpenAL
    std::string  path   = "";
    float        duration = 0;
    bool         loaded   = false;
};


/* ─────────────────────────────────────────
   SoundSource — emissor de som no mundo 3D
───────────────────────────────────────── */
struct SoundSource {
    unsigned int source = 0;   // handle OpenAL
    Vec3         position = {};
    float        volume   = 1.0f;   // 0..1
    float        pitch    = 1.0f;   // velocidade
    float        range    = 20.f;   // distância máxima audível
    bool         looping  = false;
    bool         playing  = false;
};


/* ─────────────────────────────────────────
   AudioManager
───────────────────────────────────────── */
class AudioManager {
public:

    bool init();
    void shutdown();

    /* ── Carrega sons ── */
    /* suporta: .wav .ogg */
    Sound* load(const std::string& path);
    void   unload(const std::string& path);

    /* ── Reprodução simples ── */
    void play(const std::string& path, float volume = 1.f);
    void play_3d(const std::string& path, Vec3 position, float volume = 1.f);
    void stop_all();

    /* ── Música de fundo ── */
    void play_music(const std::string& path, float volume = 0.8f);
    void stop_music();
    void set_music_volume(float volume);

    /* ── Listener (câmera/jogador) ── */
    void set_listener(Vec3 position, Vec3 forward, Vec3 up);

    /* ── Update — atualiza posições 3D ── */
    void update();

    bool is_playing_music() const { return music_source_ != 0; }

private:
    void*        device_   = nullptr;   // ALCdevice
    void*        context_  = nullptr;   // ALCcontext
    unsigned int music_source_ = 0;

    std::unordered_map<std::string, Sound> cache_;

    bool         load_wav_(const std::string& path, Sound& out);
    bool         load_ogg_(const std::string& path, Sound& out);
    unsigned int make_source_(Vec3 pos, float vol, float pitch, bool loop);
};
