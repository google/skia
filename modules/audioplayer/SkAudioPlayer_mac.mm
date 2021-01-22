/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkData.h"
#include "modules/audioplayer/SkAudioPlayer.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#include <AVFoundation/AVFoundation.h>
#endif

#ifdef SK_BUILD_FOR_IOS
// ???
#endif

class SkAudioPlayer_Mac : public SkAudioPlayer {
public:
    SkAudioPlayer_Mac(AVAudioPlayer* player, sk_sp<SkData> data)
        : fPlayer(player)
        , fData(std::move(data))
    {
        fPlayer.enableRate = YES;
        [fPlayer prepareToPlay];
    }

    ~SkAudioPlayer_Mac() override {
     //   [fPlayer release];
    }

    double onGetDuration() const override { return [fPlayer duration]; }

    double onGetTime() const override { return fPlayer.currentTime; }

    double onSetTime(double t) override {
        bool wasPlaying = this->isPlaying();
        if (wasPlaying) {
            [fPlayer pause];
        }
        fPlayer.currentTime = t;
        if (wasPlaying) {
            [fPlayer play];
        }
        return fPlayer.currentTime;
    }


    State onSetState(State state) override {
        switch (state) {
            case State::kPlaying: [fPlayer play];  break;
            case State::kStopped: [fPlayer stop];  break;
            case State::kPaused:  [fPlayer pause]; break;
        }
        return state;
    }

    float onSetRate(float r) override { fPlayer.rate = r; return r; }

    float onSetVolume(float v) override { fPlayer.volume = v; return v; }

private:
    AVAudioPlayer*  fPlayer;
    sk_sp<SkData>   fData;  // we hold this onbehalf of the player's NSData
};

std::unique_ptr<SkAudioPlayer> SkAudioPlayer::Make(sk_sp<SkData> src) {
    // The NSData does not own the actual buffer (src), but our subclass will

    NSData* data = [[NSData alloc] initWithBytesNoCopy:(void*)src->data() length:src->size()];
    AVAudioPlayer* player = [[AVAudioPlayer alloc] initWithData:data error:nil];
    [data release];

    if (player) {
        return std::unique_ptr<SkAudioPlayer>(new SkAudioPlayer_Mac(player, std::move(src)));
    }
    return nullptr;
}

#endif
