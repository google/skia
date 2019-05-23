/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/effects/SkColorMatrix.h"

static void draw_into_alpha(const SkImage* img, sk_sp<SkColorFilter> cf, const SkPixmap& dst) {
    auto canvas = SkCanvas::MakeRasterDirect(dst.info(), dst.writable_addr(), dst.rowBytes());
    canvas->scale(1.0f * dst.width() / img->width(), 1.0f * dst.height() / img->height());
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    paint.setColorFilter(cf);
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawImage(img, 0, 0, &paint);
}

static void split_into_yuv_420(const SkImage* img, SkPixmap dst[3]) {
    SkColorMatrix yuv;
    yuv.setRGB2YUV();
    float m[20];

    yuv.get20(m);
    memcpy(m + 15, m + 0, 5 * sizeof(float));   // copy Y into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[0]);

    yuv.get20(m);
    memcpy(m + 15, m + 5, 5 * sizeof(float));   // copy U into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[1]);

    yuv.get20(m);
    memcpy(m + 15, m + 10, 5 * sizeof(float));   // copy V into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[2]);
}

class YUVSplitterGM : public skiagm::GM {
    sk_sp<SkImage>  fOrig;

public:
    VideoDecoderGM() {}

protected:

    SkString onShortName() override {
        return SkString("yuv_splitter");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    void onOnceBeforeDraw() override {
        fOrig = GetResourceAsImage("images/mandrill_256.png").makeRasterImage();
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

