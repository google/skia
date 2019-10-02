/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"

// Time variants of read-pixels
//  [ colortype ][ alphatype ][ colorspace ]
//  Different combinations can trigger fast or slow paths in the impls
//
class ReadPixBench : public Benchmark {
public:
    ReadPixBench(SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : fCT(ct), fAT(at), fCS(cs)
    {
        fName.printf("readpix_%s_%s_%s",
                     at == kPremul_SkAlphaType ? "pm" : "um",
                     ct == kRGBA_8888_SkColorType ? "rgba" : "bgra",
                     cs ? "srgb" : "null");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        canvas->clear(0x80000000);

        SkISize size = canvas->getBaseLayerSize();

        auto info = SkImageInfo::Make(size, fCT, fAT, fCS);
        SkBitmap bitmap;
        bitmap.allocPixels(info);

        for (int i = 0; i < loops; i++) {
            canvas->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
        }
    }

private:
    SkColorType fCT;
    SkAlphaType fAT;
    sk_sp<SkColorSpace> fCS;
    SkString fName;
    typedef Benchmark INHERITED;
};
DEF_BENCH( return new ReadPixBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr); )
DEF_BENCH( return new ReadPixBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr); )
DEF_BENCH( return new ReadPixBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()); )
DEF_BENCH( return new ReadPixBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB()); )

DEF_BENCH( return new ReadPixBench(kBGRA_8888_SkColorType, kPremul_SkAlphaType, nullptr); )
DEF_BENCH( return new ReadPixBench(kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr); )
DEF_BENCH( return new ReadPixBench(kBGRA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()); )
DEF_BENCH( return new ReadPixBench(kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB()); )

////////////////////////////////////////////////////////////////////////////////
#include "include/core/SkBitmap.h"
#include "src/core/SkPixmapPriv.h"

class PixmapOrientBench : public Benchmark {
public:
    PixmapOrientBench() {}

protected:
    void onDelayedSetup() override {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(2048, 1024);
        fSrc.allocPixels(info);
        fSrc.eraseColor(SK_ColorBLACK);
        fDst.allocPixels(info.makeWH(info.height(), info.width()));
    }

    const char* onGetName() override {
        return "orient_pixmap";
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        SkPixmap src, dst;
        fSrc.peekPixels(&src);
        fDst.peekPixels(&dst);
        for (int i = 0; i < loops; ++i) {
            SkPixmapPriv::Orient(dst, src, kTopRight_SkEncodedOrigin);
        }
    }

private:
    SkBitmap fSrc, fDst;

    typedef Benchmark INHERITED;
};
DEF_BENCH( return new PixmapOrientBench(); )


class GetAlphafBench : public Benchmark {
    SkString fName;
    SkColorType fCT;
public:
    GetAlphafBench(SkColorType ct, const char label[]) : fCT(ct) {
        fName.printf("getalphaf_%s", label);
    }

protected:
    void onDelayedSetup() override {
        fBM.allocPixels(SkImageInfo::Make(1024, 1024, fCT, kPremul_SkAlphaType));
        fBM.eraseColor(0x88112233);
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            for (int y = 0; y < fBM.height(); ++y) {
                for (int x = 0; x < fBM.width(); ++x) {
                    fBM.getAlphaf(x, y);
                }
            }
        }
    }

private:
    SkBitmap fBM;

    typedef Benchmark INHERITED;
};
DEF_BENCH( return new GetAlphafBench(kN32_SkColorType, "rgba"); )
DEF_BENCH( return new GetAlphafBench(kRGB_888x_SkColorType, "rgbx"); )
DEF_BENCH( return new GetAlphafBench(kRGBA_F16_SkColorType, "f16"); )
DEF_BENCH( return new GetAlphafBench(kRGBA_F32_SkColorType, "f32"); )

