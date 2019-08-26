/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"

class CTConvertBench : public Benchmark {
public:
    enum class Mode { kDrawBitmap, kReadPixels };

    CTConvertBench(SkColorType from, SkColorType to, Mode m)
        : fName(SkStringPrintf("ctconvert_%d_%d_%s", from, to,
                               m == Mode::kReadPixels ? "readpixels" : "drawbitmap"))
        , fFrom(from)
        , fTo(to)
        , fMode(m) {}

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onPreDraw(SkCanvas*) override {
        if (fSrc.addr()) return;

        fSrc.alloc(SkImageInfo::Make(kBMSize, kBMSize, fFrom, kPremul_SkAlphaType));
        fSrc.erase(0x80808080);

        fDst.alloc(SkImageInfo::Make(kBMSize, kBMSize, fTo  , kPremul_SkAlphaType));

        if (fMode == Mode::kDrawBitmap) {
            fSrcBM.installPixels(fSrc);
            fDstSurf = SkSurface::MakeRasterDirect(fDst.info(), fDst.writable_addr(),
                                                   fDst.info().minRowBytes());
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        switch (fMode) {
        case Mode::kDrawBitmap: {
            SkPaint p;
            p.setBlendMode(SkBlendMode::kSrc);
            for (int i = 0; i < loops; ++i) {
                fDstSurf->getCanvas()->drawBitmap(fSrcBM, 0, 0, &p);
            }
        } break;
        case Mode::kReadPixels: {
            for (int i = 0; i < loops; ++i) {
                fSrc.readPixels(fDst);
            }
        } break;
        }
    }

private:
    static constexpr size_t kBMSize = 512;

    const SkString    fName;
    const SkColorType fFrom, fTo;
    const Mode        fMode;

    SkAutoPixmapStorage fSrc, fDst;
    SkBitmap            fSrcBM;
    sk_sp<SkSurface>    fDstSurf;

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new CTConvertBench(kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                     CTConvertBench::Mode::kDrawBitmap); )

DEF_BENCH( return new CTConvertBench(kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                     CTConvertBench::Mode::kReadPixels); )

DEF_BENCH( return new CTConvertBench(kRGBA_8888_SkColorType, kRGBA_8888_SkColorType,
                                     CTConvertBench::Mode::kDrawBitmap); )

DEF_BENCH( return new CTConvertBench(kRGBA_8888_SkColorType, kRGBA_8888_SkColorType,
                                     CTConvertBench::Mode::kReadPixels); )
