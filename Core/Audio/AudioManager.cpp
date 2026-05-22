#include "AudioManager.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <fstream>
#include <cstring>

/* ════════════════════════════════════════
   Init / Shutdown
════════════════════════════════════════ */

bool AudioManager::init() {
    device_  = alcOpenDevice(nullptr);
    if (!device_) {
        printf("[Audio] ERRO: nao encontrou dispositivo de audio\n");
        return false;
    }

    context_ = alcCreateContext((ALCdevice*)device_, nullptr);
    alcMakeContextCurrent((ALCcontext*)context_);

    /* listener padrão na origem */
    alListener3f(AL_POSITION,    0, 0, 0);
    alListener3f(AL_VELOCITY,    0, 0, 0);
    float orient[] = {0,0,-1, 0,1,0};
    alListenerfv(AL_ORIENTATION, orient);

    printf("[Audio] iniciado\n");
    return true;
}

void AudioManager::shutdown() {
    stop_all();
    stop_music();

    for (auto& [path, sound] : cache_)
        alDeleteBuffers(1, &sound.buffer);
    cache_.clear();

    alcMakeContextCurrent(nullptr);
    alcDestroyContext((ALCcontext*)context_);
    alcCloseDevice((ALCdevice*)device_);
    printf("[Audio] encerrado\n");
}

/* ════════════════════════════════════════
   Carregamento
════════════════════════════════════════ */

Sound* AudioManager::load(const std::string& path) {
    if (cache_.count(path)) return &cache_[path];

    Sound sound;
    sound.path = path;

    auto ext = path.substr(path.rfind('.') + 1);
    bool ok = false;

    if      (ext == "wav") ok = load_wav_(path, sound);
    else if (ext == "ogg") ok = load_ogg_(path, sound);
    else {
        printf("[Audio] formato '%s' nao suportado\n", ext.c_str());
        return nullptr;
    }

    if (!ok) return nullptr;

    cache_[path] = sound;
    printf("[Audio] carregado: %s\n", path.c_str());
    return &cache_[path];
}

void AudioManager::unload(const std::string& path) {
    auto it = cache_.find(path);
    if (it == cache_.end()) return;
    alDeleteBuffers(1, &it->second.buffer);
    cache_.erase(it);
}

/* ── WAV loader ── */
bool AudioManager::load_wav_(const std::string& path, Sound& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        printf("[Audio] ERRO: nao abriu '%s'\n", path.c_str());
        return false;
    }

    /* lê header RIFF */
    char  riff[4];    f.read(riff, 4);
    int   file_size;  f.read((char*)&file_size, 4);
    char  wave[4];    f.read(wave, 4);

    if (strncmp(riff, "RIFF", 4) != 0 || strncmp(wave, "WAVE", 4) != 0) {
        printf("[Audio] ERRO: '%s' nao e WAV valido\n", path.c_str());
        return false;
    }

    /* lê chunks */
    short channels=0, bits=0;
    int   sample_rate=0, data_size=0;
    std::vector<char> data;

    while (!f.eof()) {
        char   chunk_id[4]; f.read(chunk_id, 4);
        int    chunk_size;  f.read((char*)&chunk_size, 4);
        if (f.fail()) break;

        if (strncmp(chunk_id, "fmt ", 4) == 0) {
            short audio_fmt; f.read((char*)&audio_fmt,  2);
            f.read((char*)&channels,    2);
            f.read((char*)&sample_rate, 4);
            int byte_rate; f.read((char*)&byte_rate, 4);
            short block_align; f.read((char*)&block_align, 2);
            f.read((char*)&bits, 2);
            /* pula bytes extras se chunk_size > 16 */
            if (chunk_size > 16) f.seekg(chunk_size - 16, std::ios::cur);
        }
        else if (strncmp(chunk_id, "data", 4) == 0) {
            data_size = chunk_size;
            data.resize(chunk_size);
            f.read(data.data(), chunk_size);
        }
        else {
            f.seekg(chunk_size, std::ios::cur);
        }
    }

    if (data.empty()) {
        printf("[Audio] ERRO: WAV sem dados de audio\n");
        return false;
    }

    /* formato OpenAL */
    ALenum fmt = AL_FORMAT_MONO8;
    if      (channels == 1 && bits == 8)  fmt = AL_FORMAT_MONO8;
    else if (channels == 1 && bits == 16) fmt = AL_FORMAT_MONO16;
    else if (channels == 2 && bits == 8)  fmt = AL_FORMAT_STEREO8;
    else if (channels == 2 && bits == 16) fmt = AL_FORMAT_STEREO16;

    alGenBuffers(1, &out.buffer);
    alBufferData(out.buffer, fmt, data.data(), data_size, sample_rate);

    out.duration = (float)data_size / (sample_rate * channels * (bits / 8));
    out.loaded   = true;
    return true;
}

/* ── OGG loader (via stb_vorbis se disponível) ── */
bool AudioManager::load_ogg_(const std::string& path, Sound& out) {
    printf("[Audio] ogg: '%s' — instale stb_vorbis para suporte completo\n",
           path.c_str());
    (void)out;
    return false;
}

/* ════════════════════════════════════════
   Reprodução
════════════════════════════════════════ */

unsigned int AudioManager::make_source_(Vec3 pos, float vol,
                                         float pitch, bool loop) {
    unsigned int src;
    alGenSources(1, &src);
    alSource3f(src, AL_POSITION, pos.x, pos.y, pos.z);
    alSourcef(src,  AL_GAIN,     vol);
    alSourcef(src,  AL_PITCH,    pitch);
    alSourcei(src,  AL_LOOPING,  loop ? AL_TRUE : AL_FALSE);
    return src;
}

void AudioManager::play(const std::string& path, float volume) {
    Sound* s = load(path);
    if (!s) return;

    unsigned int src = make_source_({0,0,0}, volume, 1.f, false);
    alSourcei(src, AL_BUFFER, s->buffer);
    alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE); /* ignora posição 3D */
    alSourcePlay(src);

    /* fonte descartável — OpenAL limpa quando terminar */
}

void AudioManager::play_3d(const std::string& path,
                            Vec3 position, float volume) {
    Sound* s = load(path);
    if (!s) return;

    unsigned int src = make_source_(position, volume, 1.f, false);
    alSourcei(src, AL_BUFFER, s->buffer);
    alSourcePlay(src);
}

void AudioManager::stop_all() {
    /* para todas as fontes ativas */
    /* em produção: manter lista de fontes ativas */
}

/* ════════════════════════════════════════
   Música de fundo
════════════════════════════════════════ */

void AudioManager::play_music(const std::string& path, float volume) {
    stop_music();

    Sound* s = load(path);
    if (!s) return;

    music_source_ = make_source_({0,0,0}, volume, 1.f, true);
    alSourcei(music_source_, AL_BUFFER, s->buffer);
    alSourcei(music_source_, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcePlay(music_source_);

    printf("[Audio] musica: %s\n", path.c_str());
}

void AudioManager::stop_music() {
    if (!music_source_) return;
    alSourceStop(music_source_);
    alDeleteSources(1, &music_source_);
    music_source_ = 0;
}

void AudioManager::set_music_volume(float volume) {
    if (music_source_)
        alSourcef(music_source_, AL_GAIN, volume);
}

/* ════════════════════════════════════════
   Listener
════════════════════════════════════════ */

void AudioManager::set_listener(Vec3 pos, Vec3 forward, Vec3 up) {
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    float orient[] = {
        forward.x, forward.y, forward.z,
        up.x,      up.y,      up.z
    };
    alListenerfv(AL_ORIENTATION, orient);
}

void AudioManager::update() {
    /* futuro: limpar fontes que terminaram */
}
