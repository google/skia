/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "src/base/SkTime.h"

#ifndef AnimTimer_DEFINED
#define AnimTimer_DEFINED

/**
 *  Class to track a "timer". It supports 3 states: stopped, paused, and running.
 *  Playback speed is variable.
 *
 *  The caller must call updateTime() to resync with the clock (typically just before
 *  using the timer). Forcing the caller to do this ensures that the timer's return values
 *  are consistent if called repeatedly, as they only reflect the time since the last
 *  calle to updateTimer().
 */
class AnimTimer {
public:
    /**
     *  Class begins in the "stopped" state.
     */
    AnimTimer() {}

    enum State { kStopped_State, kPaused_State, kRunning_State };

    State state() const { return fState; }

    double nanos() const { return fElapsedNanos; }

    /**
     *  Control the rate at which time advances.
     */
    float getSpeed() const { return fSpeed; }
    void  setSpeed(float speed) { fSpeed = speed; }

    /**
     *  If the timer is paused or stopped, it will resume (or start if it was stopped).
     */
    void run() {
        switch (this->state()) {
            case kStopped_State:
                fPreviousNanos = SkTime::GetNSecs();
                fElapsedNanos  = 0;
                break;
            case kPaused_State:  // they want "resume"
                fPreviousNanos = SkTime::GetNSecs();
                break;
            case kRunning_State: break;
        }
        fState = kRunning_State;
    }

    void pause() {
        if (kRunning_State == this->state()) {
            fState = kPaused_State;
        }  // else stay stopped or paused
    }

    /**
     *  If the timer is stopped, start running, else it toggles between paused and running.
     */
    void togglePauseResume() {
        if (kRunning_State == this->state()) {
            this->pause();
        } else {
            this->run();
        }
    }

    /**
     *  Call this each time you want to sample the clock for the timer. This is NOT done
     *  automatically, so that repeated calls to msec() or secs() will always return the
     *  same value.
     *
     *  This may safely be called with the timer in any state.
     */
    void updateTime() {
        if (kRunning_State == this->state()) {
            double now = SkTime::GetNSecs();
            fElapsedNanos += (now - fPreviousNanos) * fSpeed;
            fPreviousNanos = now;
        }
    }

private:
    double fPreviousNanos = 0;
    double fElapsedNanos = 0;
    float  fSpeed = 1;
    State fState = kStopped_State;
};

#endif
