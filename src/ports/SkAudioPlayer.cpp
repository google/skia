/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/ports/SkAudioPlayer.h"
#include "include/private/SkFloatingPoint.h"

SkAudioPlayer::~SkAudioPlayer() {}

double SkAudioPlayer::setTime(double t) {
    double dur = this->duration();
    if (!(t >= 0 && t <= dur)) {
        t = dur;
    }
    if (t != dur) {
        State prev = this->state();
        if (prev == State::kPlaying) {
            this->pause();
        }
        t = this->onSetTime(t);
        if (prev == State::kPlaying) {
            this->play();
        }
    }
    return t;
}

double SkAudioPlayer::setNormalizedTime(double t) {
    if (t >= 0 && t <= 1) {
        this->setTime(t * this->duration());
    }
    return this->normalizedTime();
}

SkAudioPlayer::State SkAudioPlayer::setState(State s) {
    if (s != fState) {
        fState = this->onSetState(s);
    }
    return fState;
}

float SkAudioPlayer::setRate(float r) {
    if (!sk_float_isfinite(r) || r <= 0) {
        r = fRate;
    }
    if (r != fRate) {
        fRate = this->onSetRate(r);
    }
    return fRate;
}

float SkAudioPlayer::setVolume(float v) {
    if (!(v >= 0 && v <= 1)) {
        v = fVolume;
    }
    if (v != fVolume) {
        fVolume = this->onSetVolume(v);
    }
    return fVolume;
}
