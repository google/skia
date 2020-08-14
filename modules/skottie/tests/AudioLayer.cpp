/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/Skottie.h"

#include "tests/Test.h"

using namespace skottie;
using namespace skresources;

DEF_TEST(Skottie_AudioLayer, r) {
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "assets": [
               {
                 "id": "audio_0",
                 "p" : "audio.mp3",
                 "u" : "assets/"
               }
             ],
             "layers": [
               {
                 "ty"   : 6,
                 "ind"  : 0,
                 "ip"   : 20,
                 "op"   : 70,
                 "refId": "audio_0"
               },
               {
                 "ty"   : 6,
                 "ind"  : 0,
                 "ip"   : 50,
                 "op"   : 80,
                 "refId": "audio_0"
               },
               {
                 "ty": 1,
                 "ip": 0,
                 "op": 100,
                 "sw": 100,
                 "sh": 100,
                 "sc": "#ffffff"
               }
             ]
           })";

    class MockTracker final : public ExternalTrackAsset {
    public:
        bool isPlaying()    const { return fCurrentTime >= 0; }
        float currentTime() const { return fCurrentTime; }

    private:
        void seek(float t) override {
            fCurrentTime = t;
        }

        float                     fCurrentTime = 0;
    };

    class MockResourceProvider final : public ResourceProvider {
    public:
        explicit MockResourceProvider(skiatest::Reporter* r) : fReporter(r) {}

        const std::vector<sk_sp<MockTracker>>& tracks() const { return fTracks; }

    private:
        sk_sp<ExternalTrackAsset> loadAudioAsset(const char path[],
                                                 const char name[],
                                                 const char id[]) override {
            REPORTER_ASSERT(fReporter, !strcmp(path, "assets/"));
            REPORTER_ASSERT(fReporter, !strcmp(name, "audio.mp3"));
            REPORTER_ASSERT(fReporter, !strcmp(id  , "audio_0"));

            fTracks.push_back(sk_make_sp<MockTracker>());

            return fTracks.back();
        }

        skiatest::Reporter*             fReporter;
        std::vector<sk_sp<MockTracker>> fTracks;
    };

    SkMemoryStream stream(json, strlen(json));
    auto rp = sk_make_sp<MockResourceProvider>(r);

    auto skottie = skottie::Animation::Builder()
            .setResourceProvider(rp)
            .make(&stream);

    const auto& tracks = rp->tracks();

    REPORTER_ASSERT(r, skottie);
    REPORTER_ASSERT(r, tracks.size() == 2);

    skottie->seekFrame(0);
    REPORTER_ASSERT(r, !tracks[0]->isPlaying());
    REPORTER_ASSERT(r, !tracks[1]->isPlaying());

    skottie->seekFrame(20);
    REPORTER_ASSERT(r,  tracks[0]->isPlaying());
    REPORTER_ASSERT(r, !tracks[1]->isPlaying());
    REPORTER_ASSERT(r,  tracks[0]->currentTime() == 0);

    skottie->seekFrame(50);
    REPORTER_ASSERT(r, tracks[0]->isPlaying());
    REPORTER_ASSERT(r, tracks[1]->isPlaying());
    REPORTER_ASSERT(r, tracks[0]->currentTime() == 3);
    REPORTER_ASSERT(r, tracks[1]->currentTime() == 0);

    skottie->seekFrame(70);
    REPORTER_ASSERT(r, tracks[0]->isPlaying());
    REPORTER_ASSERT(r, tracks[1]->isPlaying());
    REPORTER_ASSERT(r, tracks[0]->currentTime() == 5);
    REPORTER_ASSERT(r, tracks[1]->currentTime() == 2);

    skottie->seekFrame(80);
    REPORTER_ASSERT(r, !tracks[0]->isPlaying());
    REPORTER_ASSERT(r,  tracks[1]->isPlaying());
    REPORTER_ASSERT(r,  tracks[1]->currentTime() == 3);

    skottie->seekFrame(100);
    REPORTER_ASSERT(r, !tracks[0]->isPlaying());
    REPORTER_ASSERT(r, !tracks[1]->isPlaying());
}
