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

#if 0
static void convert_rgba_to_yuva_709(SkPMColor col, uint8_t yuv[4]) {
    static const float Kr = 0.2126f;
    static const float Kb = 0.0722f;
    static const float Kg = 1.0f - Kr - Kb;

    float r = SkGetPackedR32(col) / 255.0f;
    float g = SkGetPackedG32(col) / 255.0f;
    float b = SkGetPackedB32(col) / 255.0f;

    float Ey = Kr * r + Kg * g + Kb * b;
    float Ecb = (b - Ey) / 1.8556f;
    float Ecr = (r - Ey) / 1.5748;

    yuv[0] = SkScalarRoundToInt( 219 * Ey +  16 );
    yuv[1] = SkScalarRoundToInt( 224 * Ecb + 128 );
    yuv[2] = SkScalarRoundToInt( 224 * Ecr + 128 );

    yuv[3] = SkGetPackedA32(col);
}

static void convert_pms(const SkImage* orig, SkPixmap dst[3]) {
    SkPixmap src;
    orig->peekPixels(&src);
    for (int y = 0; y < orig->height(); ++y) {
        for (int x = 0; x < orig->width(); ++x) {
            uint8_t yuva[4];
            convert_rgba_to_yuva_709(*src.addr32(x, y), yuva);
            for (int i = 0; i < 3; ++i) {
                *dst[i].writable_addr8(x, y) = yuva[i];
            }
        }
    }
}

static void scale3(float m[], float s) {
    for (int i = 0; i < 3; ++i) {
        m[i] *= s;
    }
}

static void make_rgb_to_yuv_colormatrix(float mx[20], float Kr, float Kb) {
    const float Kg = 1.0f - Kr - Kb;
    const float Su = 1 / 1.8556f;
    const float Sv = 1 / 1.5748f;

    float m[20] = {
               Kr,     Kg,        Kb,  0,  16/255.0f,
           -Kr*Su, -Kg*Su, (1-Kb)*Su,  0,  128/255.0f,
        (1-Kr)*Sv, -Kg*Sv,    -Kb*Sv,  0,  128/255.0f,
                0,      0,         0,  1,  0,
    };
    memcpy(mx, m, sizeof(m));
    scale3(mx +  0, 219/255.0f);
    scale3(mx +  5, 224/255.0f);
    scale3(mx + 10, 224/255.0f);
}
#endif

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
    auto make_scaled = [](const SkImage* img, float scale, float add) {
        float s = scale;
        float m[] = {
            s, 0, 0, 0, add,
            0, s, 0, 0, add,
            0, 0, s, 0, add,
            0, 0, 0, 1, 0,
        };
        return img->makeShader()->makeWithColorFilter(SkColorFilters::Matrix(m));
    };
    auto sh0 = make_scaled(a, -0.5f, 0.5f);
    auto sh1 = make_scaled(b, 0.5f, 0);
    SkPaint paint;
    paint.setShader(SkShaders::Blend(SkBlendMode::kPlus, sh0, sh1));
    canvas->save();
    canvas->translate(x, y);
    canvas->drawRect(SkRect::MakeWH(a->width(), a->height()), paint);
    canvas->restore();
}

class YUVSplitterGM : public skiagm::GM {
    sk_sp<SkImage>  fOrig;
    SkAutoPixmapStorage fStorage[3];
    SkPixmap        fPM[3];

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
    //  info = info.makeWH(info.width()/2, info.height()/2);
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

        canvas->drawImage(fOrig, 0, 0, nullptr);
        canvas->save();
        for (auto cs : { kRec709_SkYUVColorSpace, kRec601_SkYUVColorSpace, kJPEG_SkYUVColorSpace}) {
            split_into_yuv(fOrig.get(), cs, fPM);
            auto img = SkImage::MakeFromYUVAPixmaps(gr, cs, fPM, indices,
                                                    fPM[0].info().dimensions(),
                                                    kTopLeft_GrSurfaceOrigin,
                                                    false, false, nullptr);
            canvas->translate(fOrig->width(), 0);
            if (img) {
                canvas->drawImage(img, 0, 0, nullptr);
                draw_diff(canvas, 0, fOrig->height(), fOrig.get(), img.get());
            }
        }
        canvas->restore();
        canvas->translate(0, fOrig->height());

        canvas->drawImage(SkImage::MakeRasterCopy(fPM[0]), 0, 0, nullptr);
        canvas->drawImage(SkImage::MakeRasterCopy(fPM[1]), 0, fOrig->height(), nullptr);
        canvas->drawImage(SkImage::MakeRasterCopy(fPM[2]), fPM[1].width(), fOrig->height(), nullptr);
    }

    bool onAnimate(const AnimTimer& timer) override {
        return true;
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

