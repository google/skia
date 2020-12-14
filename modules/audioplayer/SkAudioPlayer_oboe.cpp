/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/audioplayer/SkAudioPlayer.h"

#include "include/core/SkData.h"
#include "oboe/Oboe.h"
#include "stream/MemInputStream.h"
#include "wav/WavStreamReader.h"

namespace {

class OboeAudioPlayer final : public SkAudioPlayer, oboe::AudioStreamCallback {
public:
    explicit OboeAudioPlayer(sk_sp<SkData> data)
        : fData(std::move(data))
        , fMemInputStream(const_cast<unsigned char *>
          (static_cast<const unsigned char *>(fData->data())), static_cast<int32_t>(fData->size()))
    {
      // wrap data in MemInputStream to parse WAV header
      fReader = std::make_unique<parselib::WavStreamReader>(&fMemInputStream);
      fReader->parse();
      // set member variables and builder properties using reader
      fNumSampleFrames = fReader->getNumSampleFrames();

      oboe::AudioStreamBuilder builder;
      builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
      builder.setSharingMode(oboe::SharingMode::Exclusive);
      builder.setSampleRate(fReader->getSampleRate());
      builder.setChannelCount(fReader->getNumChannels());
      builder.setCallback(this);
      builder.setFormat(oboe::AudioFormat::Float);

      // open the stream (must manually close it when done)
      fStream = nullptr;
      builder.openStream(fStream);
    }

private:
    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
        // we assume float samples here
        float *outBuffer = static_cast<float *>(audioData);
            int framesRead = fReader->getDataFloat(outBuffer, numFrames);
            fReadFrameIndex += framesRead;
            int remainingFrames = numFrames - framesRead;
            if (remainingFrames > 0) {
                if (fIsLooping) {
                    // handle wrap around
                    fReader->positionToAudio();
                    fReader->getDataFloat(&outBuffer[framesRead * fReader->getNumChannels()],
                                         remainingFrames);
                    fReadFrameIndex += remainingFrames;
                } else {
                    // render silence for rest
                    renderSilence(&outBuffer[framesRead * fReader->getNumChannels()], remainingFrames);
                    return oboe::DataCallbackResult::Stop;
                }
            }
        return oboe::DataCallbackResult::Continue;
    }

    void renderSilence(float *start, int numFrames) {
        for (int i = 0; i < numFrames * fReader->getNumChannels(); ++i) {
            start[i] = 0;
        }
    }
    double onGetDuration() const override {
        return fNumSampleFrames * fStream->getChannelCount() / fStream->getSampleRate();
    }

    double onGetTime() const override {
        return (fReadFrameIndex * fStream->getChannelCount()) / fStream->getSampleRate();
    }

    double onSetTime(double t) override {
        fReadFrameIndex = (t * fStream->getSampleRate()) / fStream->getChannelCount();
        return onGetTime();
    }

    State onSetState(State state) override {
        switch (state) {
            case State::kPlaying: fStream->start();  break;
            case State::kStopped: fStream->close();  break;
            case State::kPaused : fStream->pause(); break;
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
    std::shared_ptr<oboe::AudioStream>          fStream;
    std::unique_ptr<parselib::WavStreamReader>  fReader;
    parselib::MemInputStream                    fMemInputStream;
    int32_t                                     fReadFrameIndex {0};
    int                                         fNumSampleFrames;
    bool                                        fIsLooping {false};
};

} // namespace

std::unique_ptr<SkAudioPlayer> SkAudioPlayer::Make(sk_sp<SkData> src) {
    return std::unique_ptr<SkAudioPlayer>(new OboeAudioPlayer(std::move(src)));
}
