/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkString.h"

class WritePixelsBench : public Benchmark {
public:
    WritePixelsBench(SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : fColorType(ct)
        , fAlphaType(at)
        , fCS(std::move(cs))
        , fName("writepix")
    {
        switch (ct) {
            case kRGBA_8888_SkColorType:
                fName.append("_RGBA");
                break;
            case kBGRA_8888_SkColorType:
                fName.append("_BGRA");
                break;
            default:
                SkASSERT(0);
                break;
        }
        switch (at) {
            case kPremul_SkAlphaType:
                fName.append("_PM");
                break;
            case kUnpremul_SkAlphaType:
                fName.append("_UPM");
                break;
            default:
                SkASSERT(0);
                break;
        }
        fName.append(fCS ? "_srgb" : "_null");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkISize size = canvas->getBaseLayerSize();

        canvas->clear(0xFFFF0000);

        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), fColorType, fAlphaType,
                                             fCS);
        SkBitmap bmp;
        bmp.allocPixels(info);
        canvas->readPixels(bmp, 0, 0);

        for (int loop = 0; loop < loops; ++loop) {
            canvas->writePixels(info, bmp.getPixels(), bmp.rowBytes(), 0, 0);
        }
    }

private:
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fCS;
    SkString    fName;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr);)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());)
DEF_BENCH(return new WritePixelsBench(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());)
