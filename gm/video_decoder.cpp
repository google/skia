/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/ffmpeg/SkVideoDecoder.h"
#include "gm/gm.h"
#include "tools/Resources.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/core/SkYUVAIndex.h"
#include "include/effects/SkColorMatrix.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkYUVMath.h"

static void draw_into_alpha(const SkImage* img, sk_sp<SkColorFilter> cf, const SkPixmap& dst) {
    auto canvas = SkCanvas::MakeRasterDirect(dst.info(), dst.writable_addr(), dst.rowBytes());
    canvas->scale(1.0f * dst.width() / img->width(), 1.0f * dst.height() / img->height());
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    paint.setColorFilter(cf);
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawImage(img, 0, 0, &paint);
}

static void split_into_yuv(const SkImage* img, SkYUVColorSpace cs, const SkPixmap dst[3]) {
    float m[20];
    SkColorMatrix_RGB2YUV(cs, m);

    memcpy(m + 15, m + 0, 5 * sizeof(float));   // copy Y into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[0]);

    memcpy(m + 15, m + 5, 5 * sizeof(float));   // copy U into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[1]);

    memcpy(m + 15, m + 10, 5 * sizeof(float));   // copy V into A
    draw_into_alpha(img, SkColorFilters::Matrix(m), dst[2]);
}

static void draw_diff(SkCanvas* canvas, SkScalar x, SkScalar y,
                      const SkImage* a, const SkImage* b) {
    auto sh = SkShaders::Blend(SkBlendMode::kDifference, a->makeShader(), b->makeShader());
    SkPaint paint;
    paint.setShader(sh);
    canvas->save();
    canvas->translate(x, y);
    canvas->drawRect(SkRect::MakeWH(a->width(), a->height()), paint);

    SkColorMatrix cm;
    cm.setScale(64, 64, 64);
    paint.setShader(sh->makeWithColorFilter(SkColorFilters::Matrix(cm)));
    canvas->translate(0, a->height());
    canvas->drawRect(SkRect::MakeWH(a->width(), a->height()), paint);

    canvas->restore();
}

class YUVSplitterGM : public skiagm::GM {
    sk_sp<SkImage>      fOrig;
    SkAutoPixmapStorage fStorage[3];
    SkPixmap            fPM[3];

public:
    YUVSplitterGM() {}

protected:

    SkString onShortName() override {
        return SkString("yuv_splitter");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    void onOnceBeforeDraw() override {
        fOrig = GetResourceAsImage("images/mandrill_256.png")->makeRasterImage();

        SkImageInfo info = SkImageInfo::Make(fOrig->width(), fOrig->height(), kAlpha_8_SkColorType,
                                             kPremul_SkAlphaType);
        fStorage[0].alloc(info);
        if (0) {
            // if you want to scale U,V down by 1/2
            info = info.makeWH(info.width()/2, info.height()/2);
        }
        fStorage[1].alloc(info);
        fStorage[2].alloc(info);
        for (int i = 0; i < 3; ++i) {
            fPM[i] = fStorage[i];
        }
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* gr = canvas->getGrContext();

        SkYUVAIndex indices[4];
        indices[SkYUVAIndex::kY_Index] = {0, SkColorChannel::kR};
        indices[SkYUVAIndex::kU_Index] = {1, SkColorChannel::kR};
        indices[SkYUVAIndex::kV_Index] = {2, SkColorChannel::kR};
        indices[SkYUVAIndex::kA_Index] = {-1, SkColorChannel::kR};

        canvas->translate(fOrig->width(), 0);
        canvas->save();
        for (auto cs : { kRec709_SkYUVColorSpace, kRec601_SkYUVColorSpace, kJPEG_SkYUVColorSpace}) {
            split_into_yuv(fOrig.get(), cs, fPM);
            auto img = SkImage::MakeFromYUVAPixmaps(gr, cs, fPM, indices,
                                                    fPM[0].info().dimensions(),
                                                    kTopLeft_GrSurfaceOrigin,
                                                    false, false, nullptr);
            if (img) {
                canvas->drawImage(img, 0, 0, nullptr);
                draw_diff(canvas, 0, fOrig->height(), fOrig.get(), img.get());
            }
            canvas->translate(fOrig->width(), 0);
        }
        canvas->restore();
        canvas->translate(-fOrig->width(), 0);

        canvas->drawImage(SkImage::MakeRasterCopy(fPM[0]), 0, 0, nullptr);
        canvas->drawImage(SkImage::MakeRasterCopy(fPM[1]), 0, fPM[0].height(), nullptr);
        canvas->drawImage(SkImage::MakeRasterCopy(fPM[2]),
                          0, fPM[0].height() + fPM[1].height(), nullptr);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new YUVSplitterGM; )


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

