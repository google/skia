/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#once

#include "include/core/SkTypes.h"

class SK_API SkAudioPlayer {
public:
    virtual ~SkAudioPlayer();

    // Returns null on failure (possibly unknown format?)
    static std::unique_ptr<SkAudioPlayer> Make(sk_sp<SkData>);

    double duration() const;    // in seconds

    enum class State {
        kPlaying,
        kStopped,
        kPaused;
    };
    State state() const;
    float volume() const;       // 0...1
    float rate() const;         // 1.0 is "normal" speed

    State setState(State);      // returns actual State
    float setRate(float);       // returns actual rate
    float setVolume(float);     // returns actual volume

    void play()  { this->setState(kPlaying); }
    void pause() { this->setState(kPaused); }
    void stop()  { this->setState(kStopped); }

protected:
    SkAudioPlayer() {}    // only called by subclasses

    virtual double onGetDuration() const = 0;
    virtual State onSetState(State) = 0;
    virtual float onSetRate(float) = 0;
    virtual float onSetVolume(float) = 0;

private:
    State   fState = kStopped;
    float   fRate = 1.0f;
    float   fVolume = 1.0f;
};

#endif

