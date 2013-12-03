
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "SkString.h"

class WritePixelsBench : public SkBenchmark {
public:
    WritePixelsBench(SkCanvas::Config8888 config)
        : fConfig(config)
        , fName("writepix") {
        switch (config) {
            case SkCanvas::kNative_Premul_Config8888:
                fName.append("_native_PM");
                break;
            case SkCanvas::kNative_Unpremul_Config8888:
                fName.append("_native_UPM");
                break;
            case SkCanvas::kBGRA_Premul_Config8888:
                fName.append("_bgra_PM");
                break;
            case SkCanvas::kBGRA_Unpremul_Config8888:
                fName.append("_bgra_UPM");
                break;
            case SkCanvas::kRGBA_Premul_Config8888:
                fName.append("_rgba_PM");
                break;
            case SkCanvas::kRGBA_Unpremul_Config8888:
                fName.append("_rgba_UPM");
                break;
            default:
                SK_CRASH();
                break;
        }
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkISize size = canvas->getDeviceSize();

        canvas->clear(0xFFFF0000);

        SkBitmap bmp;
        bmp.setConfig(SkBitmap::kARGB_8888_Config, size.width(), size.height());
        canvas->readPixels(&bmp, 0, 0);

        for (int loop = 0; loop < loops; ++loop) {
            canvas->writePixels(bmp, 0, 0, fConfig);
        }
    }

private:
    SkCanvas::Config8888 fConfig;
    SkString             fName;

    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return SkNEW_ARGS(WritePixelsBench, (SkCanvas::kRGBA_Premul_Config8888)); )
DEF_BENCH( return SkNEW_ARGS(WritePixelsBench, (SkCanvas::kRGBA_Unpremul_Config8888)); )
