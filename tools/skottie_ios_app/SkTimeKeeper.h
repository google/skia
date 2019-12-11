// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkTimeKeeper_DEFINED
#define SkTimeKeeper_DEFINED

#include "include/core/SkTime.h"

#include <cmath>

class SkTimeKeeper {
private:
    double fStartTime = 0; // used when running
    float fAnimationMoment = 0; // when paused.
    float fDuration = 0;
    bool fPaused = false;
    bool fStopAtEnd = false;

public:
    void setStopAtEnd(bool s) { fStopAtEnd = s; }

    float currentTime() {
        if (0 == fDuration) {
            return 0;
        }
        if (fPaused) {
            return fAnimationMoment;
        }
        double time = 1e-9 * (SkTime::GetNSecs() - fStartTime);
        if (fStopAtEnd && time >= fDuration) {
            fPaused = true;
            fAnimationMoment = fDuration;
            return fAnimationMoment;
        }
        return std::fmod(time, fDuration);
    }

    void setDuration(float d) {
        fDuration = d;
        fStartTime = SkTime::GetNSecs();
        fAnimationMoment = 0;
    }

    bool paused() const { return fPaused; }

    float duration() const { return fDuration; }

    void seek(float seconds) {
        if (fPaused) {
            fAnimationMoment = std::fmod(seconds, fDuration);
        } else {
            fStartTime = SkTime::GetNSecs() - 1e9 * seconds;
        }
    }

    void togglePaused() {
        if (fPaused) {
            double offset = (fAnimationMoment >= fDuration) ? 0 : -1e9 * fAnimationMoment;
            fStartTime = SkTime::GetNSecs() + offset;
            fPaused = false;
        } else {
            fAnimationMoment = this->currentTime();
            fPaused = true;
        }
    }
};
#endif  // SkTimeKeeper_DEFINED
