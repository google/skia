/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkData.h"
#include "modules/audioplayer/SkAudioPlayer.h"

#include <SFML/Audio.hpp>

namespace {

class SFMLAudioPlayer final : public SkAudioPlayer {
public:
    explicit SFMLAudioPlayer(sk_sp<SkData> data)
        : fData(std::move(data))
    {
        fMusic.openFromMemory(fData->data(), fData->size());
    }

private:
    double onGetDuration() const override {
        return static_cast<double>(fMusic.getDuration().asSeconds());
    }

    double onGetTime() const override {
        return static_cast<double>(fMusic.getPlayingOffset().asSeconds());
    }

    double onSetTime(double t) override {
        fMusic.setPlayingOffset(sf::seconds(static_cast<float>(t)));

        return this->onGetTime();
    }

    State onSetState(State state) override {
        switch (state) {
            case State::kPlaying: fMusic.play();  break;
            case State::kStopped: fMusic.stop();  break;
            case State::kPaused : fMusic.pause(); break;
        }

        return state;
    }

    float onSetRate(float r) override {
        fMusic.setPitch(r);
        return r;
    }

    float onSetVolume(float v) override {
        fMusic.setVolume(v * 100);
        return v;
    }

    const sk_sp<SkData> fData;
    sf::Music           fMusic;
};

} // namespace

std::unique_ptr<SkAudioPlayer> SkAudioPlayer::Make(sk_sp<SkData> src) {
    auto player = std::make_unique<SFMLAudioPlayer>(std::move(src));

    return player->duration() > 0 ? std::move(player) : nullptr;
}
