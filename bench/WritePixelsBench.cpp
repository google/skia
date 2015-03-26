/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkString.h"

class WritePixelsBench : public Benchmark {
public:
    WritePixelsBench(SkColorType ct, SkAlphaType at)
        : fColorType(ct)
        , fAlphaType(at)
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
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        SkISize size = canvas->getDeviceSize();

        canvas->clear(0xFFFF0000);

        SkBitmap bmp;
        bmp.allocN32Pixels(size.width(), size.height());
        canvas->readPixels(&bmp, 0, 0);

        SkImageInfo info = SkImageInfo::Make(bmp.width(), bmp.height(), fColorType, fAlphaType);

        for (int loop = 0; loop < loops; ++loop) {
            canvas->writePixels(info, bmp.getPixels(), bmp.rowBytes(), 0, 0);
        }
    }

private:
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    SkString    fName;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return SkNEW_ARGS(WritePixelsBench, (kRGBA_8888_SkColorType, kPremul_SkAlphaType)); )
DEF_BENCH( return SkNEW_ARGS(WritePixelsBench, (kRGBA_8888_SkColorType, kUnpremul_SkAlphaType)); )
