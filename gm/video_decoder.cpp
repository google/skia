/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoDecoder.h"
#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

#include "include/utils/SkRandom.h"
static void test_encoder() {
    SkVideoEncoder enc;
    SkImageInfo info = SkImageInfo::MakeN32Premul(640, 480);

    if (enc.beginRecording(info, 25)) {
        SkRandom rand;
        float a[4] = {1, 0, 0, 1};
        float b[4] = {0, 0, 1, 1};
        int dur = 25*3;
        for (int i = 0; i < dur; ++i) {
            SkCanvas* canvas = enc.beginFrame();

            float t = 1.0 * i / dur;
            float c[4];
            for (int j = 0; j < 4; ++j) {
                c[j] = a[j] * (1 - t) + b[j] * t;
            }
            SkColor4f color;
            memcpy(&color.fR, c, sizeof(c));
            SkPaint paint;
            paint.setColor(color);
            canvas->drawPaint(paint);
            enc.endFrame();
        }

        if (auto data = enc.endRecording()) {
            SkFILEWStream stream("/skia/tmp.mp4");
            stream.write(data->data(), data->size());
        }
    }
}

const char gFilename[] = "/skia/tmp.mp4";
//const char gFilename[] = "/skia/ice.mp4";

class VideoDecoderGM : public skiagm::GM {
    SkVideoDecoder fDecoder;

public:
    VideoDecoderGM() {}

protected:

    SkString onShortName() override {
        return SkString("videodecoder");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    void onOnceBeforeDraw() override {
        if (1) test_encoder();

        if (!fDecoder.loadStream(SkStream::MakeFromFile(gFilename))) {
            SkDebugf("could not load movie file\n");
        }
        SkDebugf("duration %g\n", fDecoder.duration());
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* gr = canvas->getGrContext();
        if (!gr) {
            return;
        }

        fDecoder.setGrContext(gr); // gr can change over time in viewer

        double timeStamp;
        auto img = fDecoder.nextImage(&timeStamp);
        if (!img) {
            (void)fDecoder.rewind();
            img = fDecoder.nextImage(&timeStamp);
        }
        if (img) {
            if (0) {
                SkDebugf("ts %g\n", timeStamp);
            }
            canvas->drawImage(img, 10, 10, nullptr);
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        return true;
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new VideoDecoderGM; )

