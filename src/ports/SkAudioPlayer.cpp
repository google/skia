/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/ports/SkAudioPlayer.h"
#include "include/private/SkFloatingPoint.h"

SkAudioPlayer::~SkAudioPlayer() {}

SkAudioPlayer::State SkAudioPlayer::setState(State s) {
    if (s != fState) {
        s = this->onSetState(s);
    }
    return s;
}

float SkAudioPlayer::setRate(float r) {
    if (!sk_float_isfinite(r) || r <= 0) {
        r = fRate;
    }
    if (r != fRate) {
        r = this->onSetRate(r);
    }
    return r;
}

float SkAudioPlayer::setVolume(float v) {
    if (!(v >= 0 && v <= 1)) {
        v = fVolume;
    }
    if (v != fVolume) {
        v = this->onSetVolume(v);
    }
    return v;
}
