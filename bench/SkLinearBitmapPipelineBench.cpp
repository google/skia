/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include "SkColor.h"
#include "SkLinearBitmapPipeline.h"
#include "SkBitmapProcShader.h"
#include "SkPM4f.h"
#include "Benchmark.h"
#include "SkShader.h"
#include "SkImage.h"

struct CommonBitmapFPBenchmark : public Benchmark {
    CommonBitmapFPBenchmark(
        SkISize srcSize,
        bool isSRGB,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
        : fIsSRGB(isSRGB)
        , fM{m}
        , fUseBilerp{useBilerp}
        , fXTile{xTile}
        , fYTile{yTile} {
        fSrcSize = srcSize;
    }

    static SkString tileName(const char* pre, SkShader::TileMode mode) {
        SkString name{pre};
        switch (mode) {
            case SkShader::kClamp_TileMode:
                name.append("Clamp");
                return name;
            case SkShader::kRepeat_TileMode:
                name.append("Repeat");
                return name;
            case SkShader::kMirror_TileMode:
                name.append("Mirror");
                return name;
            default:
                name.append("Unknown");
                return name;
        }
    }

    const char* onGetName() override {
        fName.set("SkBitmapFP");
        if (fM.getType() & SkMatrix::kPerspective_Mask) {
            fName.append("Perspective");
        } else if (fM.getType() & SkMatrix::kAffine_Mask) {
            fName.append("Affine");
        } else if (fM.getType() & SkMatrix::kScale_Mask) {
            fName.append("Scale");
        } else if (fM.getType() & SkMatrix::kTranslate_Mask) {
            fName.append("Translate");
        } else {
            fName.append("Identity");
        }

        fName.append(tileName("X", fXTile));
        fName.append(tileName("Y", fYTile));

        if (fUseBilerp) {
            fName.append("Filter");
        } else {
            fName.append("Nearest");
        }

        fName.appendf("%s", BaseName().c_str());

        return fName.c_str();
    }

    void onPreDraw(SkCanvas*) override {
        int width = fSrcSize.fWidth;
        int height = fSrcSize.fHeight;
        fBitmap.reset(new uint32_t[width * height]);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                fBitmap[y * width + x] = (y << 8) + x + (128<<24);
            }
        }

        bool trash = fM.invert(&fInvert);
        sk_ignore_unused_variable(trash);

        fInfo = SkImageInfo::MakeN32Premul(width, height, fIsSRGB ?
                                       SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named) : nullptr);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual SkString BaseName() = 0;

    SkString fName;
    SkISize fSrcSize;
    bool fIsSRGB;
    SkMatrix fM;
    SkMatrix fInvert;
    bool fUseBilerp;
    SkShader::TileMode fXTile;
    SkShader::TileMode fYTile;
    SkImageInfo fInfo;
    std::unique_ptr<uint32_t[]> fBitmap;
};

struct SkBitmapFPGeneral final : public CommonBitmapFPBenchmark {
    SkBitmapFPGeneral(
        SkISize srcSize,
        bool isSRGB,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
            : CommonBitmapFPBenchmark(srcSize, isSRGB, m, useBilerp, xTile, yTile) { }

    SkString BaseName() override {
        SkString name;
        if (fInfo.gammaCloseToSRGB()) {
            name.set("sRGB");
        } else {
            name.set("Linr");
        }
        return name;
    }

    void onDraw(int loops, SkCanvas*) override {
        int width = fSrcSize.fWidth;
        int height = fSrcSize.fHeight;

        SkAutoTMalloc<SkPM4f> FPbuffer(width*height);

        SkFilterQuality filterQuality;
        if (fUseBilerp) {
            filterQuality = SkFilterQuality::kLow_SkFilterQuality;
        } else {
            filterQuality = SkFilterQuality::kNone_SkFilterQuality;
        }

        SkPixmap srcPixmap{fInfo, fBitmap.get(), static_cast<size_t>(4 * width)};

        SkLinearBitmapPipeline pipeline{
            fInvert, filterQuality, fXTile, fYTile, SK_ColorBLACK, srcPixmap};

        int count = 100;

        for (int n = 0; n < 1000*loops; n++) {
            pipeline.shadeSpan4f(3, 6, FPbuffer, count);
        }
    }
};

struct SkBitmapFPOrigShader : public CommonBitmapFPBenchmark {
    SkBitmapFPOrigShader(
        SkISize srcSize,
        bool isSRGB,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
            : CommonBitmapFPBenchmark(srcSize, isSRGB, m, useBilerp, xTile, yTile) { }

    SkString BaseName() override {
        SkString name{"Orig"};
        return name;
    }

    void onPreDraw(SkCanvas* c) override {
        CommonBitmapFPBenchmark::onPreDraw(c);

        fImage = SkImage::MakeRasterCopy(
            SkPixmap(fInfo, fBitmap.get(), sizeof(SkPMColor) * fSrcSize.fWidth));
        fPaint.setShader(fImage->makeShader(fXTile, fYTile));
        if (fUseBilerp) {
            fPaint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
        } else {
            fPaint.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
        }
    }

    void onPostDraw(SkCanvas*) override {

    }

    void onDraw(int loops, SkCanvas*) override {
        int width = fSrcSize.fWidth;
        int height = fSrcSize.fHeight;

        SkAutoTMalloc<SkPMColor> buffer4b(width*height);

        uint32_t storage[kSkBlitterContextSize];
        const SkShader::ContextRec rec(fPaint, fM, nullptr,
                                       SkShader::ContextRec::kPMColor_DstType);
        SkASSERT(fPaint.getShader()->contextSize(rec) <= sizeof(storage));
        SkShader::Context* ctx = fPaint.getShader()->createContext(rec, storage);

        int count = 100;

        for (int n = 0; n < 1000*loops; n++) {
            ctx->shadeSpan(3, 6, buffer4b, count);
        }

        ctx->~Context();
    }
    SkPaint fPaint;
    sk_sp<SkImage> fImage;
};

const bool gSRGB = true;
const bool gLinearRGB = false;
static SkISize srcSize = SkISize::Make(120, 100);
static SkMatrix mI = SkMatrix::I();
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

static SkMatrix mS = SkMatrix::MakeScale(2.7f, 2.7f);
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

// Repeat
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mS, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mS, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mS, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mS, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mS, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mS, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

static SkMatrix rotate(SkScalar r) {
    SkMatrix m;
    m.setRotate(30);
    return m;
}

static SkMatrix mR = rotate(30);
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

// Repeat
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mR, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mR, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mR, false,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gSRGB, mR, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, gLinearRGB, mR, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, gLinearRGB, mR, true,
    SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);)
