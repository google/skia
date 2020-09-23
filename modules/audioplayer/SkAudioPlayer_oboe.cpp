/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkData.h"
#include "modules/audioplayer/SkAudioPlayer.h"
#include "oboe/Oboe.h"
#include "wav/WavStreamReader.h"
#include "stream/MemInputStream.h"

namespace {

class OboeAudioPlayer final : public SkAudioPlayer, oboe::AudioStreamCallback {
public:
    explicit OboeAudioPlayer(sk_sp<SkData> data)
        : fData(std::move(data))
    {
      // wrap data in MemInputStream to parse WAV header
      parselib::MemInputStream inputStream {const_cast<unsigned char *>(static_cast<const unsigned char *>(fData->data())),
                                              static_cast<int32_t>(fData->size())};
      reader = std::make_unique<parselib::WavStreamReader>(
            parselib::WavStreamReader{&inputStream});
      reader->parse();
      // set member variables and builder properties using reader
      mReadFrameIndex = 0;
      mNumSampleFrames = reader->getNumSampleFrames();

      oboe::AudioStreamBuilder builder;
      builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
      builder.setSharingMode(oboe::SharingMode::Exclusive);
      builder.setSampleRate(reader->getSampleRate());
      builder.setChannelCount(reader->getNumChannels());
      builder.setCallback(this);

      // open the stream (must manually close it when done)
      stream = nullptr;
      oboe::Result result = builder.openStream(&stream);
    }

    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
        // we assume float samples here
        float *outBuffer = static_cast<float *>(audioData);
            int framesRead = reader->getDataFloat(outBuffer, numFrames);
            mReadFrameIndex += framesRead;
            int remainingFrames = numFrames - framesRead;
            if (remainingFrames > 0) {
                if (mIsLooping) {
                    // handle wrap around
                    reader->positionToAudio();
                    reader->getDataFloat(&outBuffer[framesRead * reader->getNumChannels()],
                                         remainingFrames);
                    mReadFrameIndex += remainingFrames;
                } else {
                    // render silence for rest
                    renderSilence(&outBuffer[framesRead * reader->getNumChannels()], remainingFrames);
                    return oboe::DataCallbackResult::Stop;
                }
            }
        return oboe::DataCallbackResult::Continue;
    }

private:

    void renderSilence(float *start, int numFrames) {
        for (int i = 0; i < numFrames * reader->getNumChannels(); ++i) {
            start[i] = 0;
        }
    }
    double onGetDuration() const override {
        return mNumSampleFrames * stream->getChannelCount() / stream->getSampleRate();
    }

    double onGetTime() const override {
        return (mReadFrameIndex * stream->getChannelCount()) / stream->getSampleRate();
    }

    double onSetTime(double t) override {
        mReadFrameIndex = (t * stream->getSampleRate()) / stream->getChannelCount();
        return onGetTime();
    }

    State onSetState(State state) override {
        switch (state) {
            case State::kPlaying: stream->start();  break;
            case State::kStopped: stream->close();  break;
            case State::kPaused : stream->pause(); break;
        }

        return state;
    }


    // TODO: implement rate function (change sample rate of AudioStream)
    float onSetRate(float r) override {
        return r;
    }

    // TODO: implement volume function (multiply each sample by desired amplitude)
    float onSetVolume(float v) override {
        return v;
    }

    const sk_sp<SkData>                         fData;
    oboe::AudioStream                           *stream;
    std::unique_ptr<parselib::WavStreamReader>  reader;
    int32_t                                     mReadFrameIndex;
    int                                         mNumSampleFrames;
    bool                                        mIsLooping {false};
};

} // namespace

std::unique_ptr<SkAudioPlayer> SkAudioPlayer::Make(sk_sp<SkData> src) {
    return std::unique_ptr<SkAudioPlayer>(new OboeAudioPlayer(std::move(src)));
}
