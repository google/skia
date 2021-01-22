/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/audioplayer/SkAudioPlayer.h"
#include <algorithm>
#include <cmath>

SkAudioPlayer::~SkAudioPlayer() {}

double SkAudioPlayer::setTime(double t) {
    t = std::min(std::max(t, 0.0), this->duration());
    if (!std::isfinite(t)) {
        t = this->time();
    }
    if (t != this->time()) {
        t = this->onSetTime(t);
    }
    return t;
}

double SkAudioPlayer::setNormalizedTime(double t) {
    this->setTime(t * this->duration());
    return this->normalizedTime();
}

SkAudioPlayer::State SkAudioPlayer::setState(State s) {
    if (s != fState) {
        fState = this->onSetState(s);
    }
    return fState;
}

float SkAudioPlayer::setRate(float r) {
    r = std::min(std::max(r, 0.f), 1.f);
    if (!std::isfinite(r)) {
        r = fRate;
    }
    if (r != fRate) {
        fRate = this->onSetRate(r);
    }
    return fRate;
}

float SkAudioPlayer::setVolume(float v) {
    v = std::min(std::max(v, 0.f), 1.f);
    if (!std::isfinite(v)) {
        v = fVolume;
    }
    if (v != fVolume) {
        fVolume = this->onSetVolume(v);
    }
    return fVolume;
}
