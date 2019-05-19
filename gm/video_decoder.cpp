/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoDecoder.h"
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

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
        if (!fDecoder.loadStream(SkStream::MakeFromFile("/skia/ice.mp4"))) {
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

