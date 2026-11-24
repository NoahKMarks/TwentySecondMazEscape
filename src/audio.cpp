#include "audio.h"
#include <string>

namespace {
constexpr const char* audioDirectory = "resources/audio/";
constexpr const char* musicFilename = "music.mp3";
constexpr std::array<const char*, static_cast<unsigned int>(SoundEffect::Count)> effectFilenames = {
    "move.wav",    // Move: metal footstep
    "turn.wav",    // Turn
    "blocked.wav", // Blocked: shield slam
    "portal.wav",  // Portal
    "win.wav"      // Win: achievement unlocked
};
constexpr float musicVolume = 0.25f;
constexpr float musicSpeed = 2.0f/3.0f;
constexpr float effectsVolume = 0.5f;
}

void GameAudio::initialize()
{
    if (ready_) return;

    InitAudioDevice();
    ready_ = IsAudioDeviceReady();
    if (!ready_) {
        TraceLog(LOG_WARNING, "Game audio unavailable; continuing without sound");
        return;
    }

    SetMasterVolume(muted_ ? 0.0f : 1.0f);
    const std::string musicPath = std::string(audioDirectory) + musicFilename;
    if (FileExists(musicPath.c_str())) {
        music_ = LoadMusicStream(musicPath.c_str());
        if (IsMusicValid(music_)) {
            music_.looping = true;
            SetMusicVolume(music_, musicVolume);
            SetMusicPitch(music_, musicSpeed);
        }
    } else {
        TraceLog(LOG_WARNING, "Optional music file missing: %s", musicPath.c_str());
    }

    for (unsigned int i = 0; i < effects_.size(); ++i) {
        const std::string effectPath = std::string(audioDirectory) + effectFilenames[i];
        if (FileExists(effectPath.c_str())) {
            effects_[i] = LoadSound(effectPath.c_str());
            if (IsSoundValid(effects_[i])) SetSoundVolume(effects_[i], effectsVolume);
        } else {
            TraceLog(LOG_WARNING, "Optional sound file missing: %s", effectPath.c_str());
        }
    }
}

void GameAudio::startMusic()
{
    if (ready_ && IsMusicValid(music_) && !IsMusicStreamPlaying(music_)) {
        PlayMusicStream(music_);
    }
}

void GameAudio::update()
{
    // Stream buffers must be refreshed every frame, even while muted.
    if (ready_ && IsMusicValid(music_)) UpdateMusicStream(music_);
}

void GameAudio::play(SoundEffect effect)
{
    const auto index = static_cast<unsigned int>(effect);
    if (ready_ && !muted_ && index < effects_.size() && IsSoundValid(effects_[index])) {
        PlaySound(effects_[index]);
    }
}

void GameAudio::setMuted(bool muted)
{
    muted_ = muted;
    if (ready_) SetMasterVolume(muted_ ? 0.0f : 1.0f);
}

void GameAudio::toggleMute()
{
    setMuted(!muted_);
}

void GameAudio::shutdown()
{
    if (!ready_) return;

    for (Sound effect : effects_) {
        if (IsSoundValid(effect)) UnloadSound(effect);
    }
    if (IsMusicValid(music_)) UnloadMusicStream(music_);
    effects_ = {};
    music_ = {};
    CloseAudioDevice();
    ready_ = false;
}
