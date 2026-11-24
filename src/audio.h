#pragma once

#include "raylib.h"
#include <array>

enum class SoundEffect {
    Move,
    Turn,
    Blocked,
    Portal,
    Win,
    Count
};

// Owns the game's audio device and assets. Missing audio leaves the game playable.
class GameAudio {
public:
    GameAudio() = default;
    GameAudio(const GameAudio&) = delete;
    GameAudio& operator=(const GameAudio&) = delete;

    void initialize();
    void startMusic();
    void update();
    void play(SoundEffect effect);
    void setMuted(bool muted);
    void toggleMute();
    void shutdown();
    bool isReady() const { return ready_; }
    bool isMuted() const { return muted_; }

private:
    Music music_{};
    std::array<Sound, static_cast<unsigned int>(SoundEffect::Count)> effects_{};
    bool ready_ = false;
    bool muted_ = false;
};
