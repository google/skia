/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTime.h"

#ifndef SkAnimTimer_DEFINED
#define SkAnimTimer_DEFINED

/**
 *  Class to track a "timer". It supports 3 states: stopped, paused, running.
 *
 *  The caller must call updateTime() to resync with the clock (typically just before
 *  using the timer). Forcing the caller to do this ensures that the timer's return values
 *  are consistent if called repeatedly, as they only reflect the time since the last
 *  calle to updateTimer().
 */
class SkAnimTimer {
public:
    enum State {
        kStopped_State,
        kPaused_State,
        kRunning_State
    };

    /**
     *  Class begins in the "stopped" state.
     */
    SkAnimTimer() : fBaseTimeNanos(0), fCurrTimeNanos(0), fState(kStopped_State) {}

    bool isStopped() const { return kStopped_State == fState; }
    bool isRunning() const { return kRunning_State == fState; }
    bool isPaused() const { return kPaused_State == fState; }

    /**
     *  Stops the timer, and resets it, such that the next call to run or togglePauseResume
     *  will begin at time 0.
     */
    void stop() {
        this->setState(kStopped_State);
    }

    /**
     *  If the timer is paused or stopped, it will resume (or start if it was stopped).
     */
    void run() {
        this->setState(kRunning_State);
    }

    /**
     *  If the timer is stopped, this has no effect, else it toggles between paused and running.
     */
    void togglePauseResume() {
        if (kRunning_State == fState) {
            this->setState(kPaused_State);
        } else {
            this->setState(kRunning_State);
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
        if (kRunning_State == fState) {
            fCurrTimeNanos = SkTime::GetNSecs();
        }
    }

    /**
     *  Return the time in milliseconds the timer has been in the running state.
     *  Returns 0 if the timer is stopped. Behavior is undefined if the timer
     *  has been running longer than SK_MSecMax.
     */
    SkMSec msec() const {
        const double msec = (fCurrTimeNanos - fBaseTimeNanos) * 1e-6;
        SkASSERT(SK_MSecMax >= msec);
        return static_cast<SkMSec>(msec);
    }

    /**
     *  Return the time in seconds the timer has been in the running state.
     *  Returns 0 if the timer is stopped.
     */
    double secs() const { return (fCurrTimeNanos - fBaseTimeNanos) * 1e-9; }

    /**
     *  Return the time in seconds the timer has been in the running state,
     *  scaled by "speed" and (if not zero) mod by period.
     *  Returns 0 if the timer is stopped.
     */
    SkScalar scaled(SkScalar speed, SkScalar period = 0) const {
        double value = this->secs() * speed;
        if (period) {
            value = ::fmod(value, SkScalarToDouble(period));
        }
        return SkDoubleToScalar(value);
    }

    /**
     * Transitions from ends->mid->ends linearly over period seconds. The phase specifies a phase
     * shift in seconds.
     */
    SkScalar pingPong(SkScalar period, SkScalar phase, SkScalar ends, SkScalar mid) const {
        return PingPong(this->secs(), period, phase, ends, mid);
    }

    /** Helper for computing a ping-pong value without a SkAnimTimer object. */
    static SkScalar PingPong(double t, SkScalar period, SkScalar phase, SkScalar ends,
                             SkScalar mid) {
        double value = ::fmod(t + phase, period);
        double half = period / 2.0;
        double diff = ::fabs(value - half);
        return SkDoubleToScalar(ends + (1.0 - diff / half) * (mid - ends));
    }

private:
    double  fBaseTimeNanos;
    double  fCurrTimeNanos;
    State   fState;

    void setState(State newState) {
        switch (newState) {
            case kStopped_State:
                fBaseTimeNanos = fCurrTimeNanos = 0;
                fState = kStopped_State;
                break;
            case kPaused_State:
                if (kRunning_State == fState) {
                    fState = kPaused_State;
                } // else stay stopped or paused
                break;
            case kRunning_State:
                switch (fState) {
                    case kStopped_State:
                        fBaseTimeNanos = fCurrTimeNanos = SkTime::GetNSecs();
                        break;
                    case kPaused_State: {// they want "resume"
                        double now = SkTime::GetNSecs();
                        fBaseTimeNanos += now - fCurrTimeNanos;
                        fCurrTimeNanos = now;
                    } break;
                    case kRunning_State:
                        break;
                }
                fState = kRunning_State;
                break;
        }
    }
};

#endif
