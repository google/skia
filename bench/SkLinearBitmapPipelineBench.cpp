/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include "SkColor.h"
#include "SkLinearBitmapPipeline.h"
#include "Benchmark.h"
#include "SkShader.h"
#include "SkImage.h"

struct CommonBitmapFPBenchmark : public Benchmark {
    CommonBitmapFPBenchmark(
        SkISize srcSize,
        SkColorProfileType colorProfile,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
        : fColorProfile(colorProfile)
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
        SkString name {"SkBitmapFP"};
        if (fM.getType() & SkMatrix::kPerspective_Mask) {
            name.append("Perspective");
        } else if (fM.getType() & SkMatrix::kAffine_Mask) {
            name.append("Affine");
        } else if (fM.getType() & SkMatrix::kScale_Mask) {
            name.append("Scale");
        } else if (fM.getType() & SkMatrix::kTranslate_Mask) {
            name.append("Translate");
        } else {
            name.append("Identity");
        }

        name.append(tileName("X", fXTile));
        name.append(tileName("Y", fYTile));

        if (fUseBilerp) {
            name.append("Filter");
        } else {
            name.append("Nearest");
        }

        name.appendf("%s", BaseName().c_str());

        return name.c_str();
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

        fInfo = SkImageInfo::MakeN32Premul(width, height, fColorProfile);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual SkString BaseName() = 0;

    SkISize fSrcSize;
    SkColorProfileType fColorProfile;
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
        SkColorProfileType colorProfile,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
            : CommonBitmapFPBenchmark(srcSize, colorProfile, m, useBilerp, xTile, yTile) { }
    SkString BaseName() override {
        SkString name;
        if (fInfo.isSRGB()) {
            name.set("sRGB");
        } else {
            name.set("Linr");
        }
        return name;
    }

    void onDraw(int loops, SkCanvas*) override {
        int width = fSrcSize.fWidth;
        int height = fSrcSize.fHeight;

        SkPM4f* FPbuffer = new SkPM4f[width * height];

        SkLinearBitmapPipeline pipeline{fInvert, fXTile, fYTile, fInfo, fBitmap.get(), };

        int count = 100;

        for (int n = 0; n < 1000*loops; n++) {
            pipeline.shadeSpan4f(3, 6, FPbuffer, count);
        }

        delete [] FPbuffer;

    }
};

struct SkBitmapFPOrigShader : public CommonBitmapFPBenchmark {
    SkBitmapFPOrigShader(
        SkISize srcSize,
        SkColorProfileType colorProfile,
        SkMatrix m,
        bool useBilerp,
        SkShader::TileMode xTile,
        SkShader::TileMode yTile)
            : CommonBitmapFPBenchmark(srcSize, colorProfile, m, useBilerp, xTile, yTile) { }
    SkString BaseName() override {
        SkString name{"Orig"};
        return name;
    }

    void onPreDraw(SkCanvas* c) override {
        CommonBitmapFPBenchmark::onPreDraw(c);

        SkImage* image = SkImage::NewRasterCopy(
            fInfo, fBitmap.get(), sizeof(SkPMColor) * fSrcSize.fWidth);
        fImage.reset(image);
        SkShader* shader = fImage->newShader(fXTile, fYTile);
        if (fUseBilerp) {
            fPaint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);
        } else {
            fPaint.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
        }
        fPaint.setShader(shader)->unref();

    }

    void onPostDraw(SkCanvas*) override {

    }

    void onDraw(int loops, SkCanvas*) override {
        int width = fSrcSize.fWidth;
        int height = fSrcSize.fHeight;

        SkPMColor *buffer4b = new SkPMColor[width * height];

        uint32_t storage[200];
        SkASSERT(fPaint.getShader()->contextSize() <= sizeof(storage));
        SkShader::Context* ctx = fPaint.getShader()->createContext(
            {fPaint, fM, nullptr},
            storage);

        int count = 100;

        for (int n = 0; n < 1000*loops; n++) {
            ctx->shadeSpan(3, 6, buffer4b, count);
        }

        ctx->~Context();
        delete buffer4b;
    }
    SkPaint fPaint;
    SkAutoTUnref<SkImage> fImage;
};

static SkISize srcSize = SkISize::Make(120, 100);
static SkMatrix mI = SkMatrix::I();
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mI, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mI, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

static SkMatrix mS = SkMatrix::MakeScale(2.7f, 2.7f);
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mS, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mS, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

static SkMatrix rotate(SkScalar r) {
    SkMatrix m;
    m.setRotate(30);
    return m;
}

static SkMatrix mR = rotate(30);
DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mR, false,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kSRGB_SkColorProfileType, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPGeneral(
    srcSize, kLinear_SkColorProfileType, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

DEF_BENCH(return new SkBitmapFPOrigShader(
    srcSize, kLinear_SkColorProfileType, mR, true,
    SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);)

