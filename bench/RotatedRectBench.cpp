/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlendModePriv.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#include <ctype.h>

/** This benchmark tests rendering rotated rectangles. It can optionally apply AA and/or change the
    paint color between each rect in different ways using the ColorType enum. The xfermode used can
    be specified as well.
  */

enum ColorType {
    kConstantOpaque_ColorType,
    kConstantTransparent_ColorType,
    kChangingOpaque_ColorType,
    kChangingTransparent_ColorType,
    kAlternatingOpaqueAndTransparent_ColorType,
};

static inline SkColor start_color(ColorType ct) {
    switch (ct) {
        case kConstantOpaque_ColorType:
        case kChangingOpaque_ColorType:
        case kAlternatingOpaqueAndTransparent_ColorType:
            return 0xFFA07040;
        case kConstantTransparent_ColorType:
        case kChangingTransparent_ColorType:
            return 0x80A07040;
    }
    SkFAIL("Shouldn't reach here.");
    return 0;
}

static inline SkColor advance_color(SkColor old, ColorType ct, int step) {
    if (kAlternatingOpaqueAndTransparent_ColorType == ct) {
        ct = (step & 0x1) ? kChangingOpaque_ColorType : kChangingTransparent_ColorType ;
    }
    switch (ct) {
        case kConstantOpaque_ColorType:
        case kConstantTransparent_ColorType:
            return old;
        case kChangingOpaque_ColorType:
            return 0xFF000000 | (old + 0x00010307);
        case kChangingTransparent_ColorType:
            return (0x00FFFFFF & (old + 0x00010307)) | 0x80000000;
        case kAlternatingOpaqueAndTransparent_ColorType:
            SkFAIL("Can't get here");
    }
    SkFAIL("Shouldn't reach here.");
    return 0;
}

static SkString to_lower(const char* str) {
    SkString lower(str);
    for (size_t i = 0; i < lower.size(); i++) {
        lower[i] = tolower(lower[i]);
    }
    return lower;
}

class RotRectBench: public Benchmark {
public:
    RotRectBench(bool aa, ColorType ct, SkBlendMode mode)
        : fAA(aa)
        , fColorType(ct)
        , fMode(mode) {
        this->makeName();
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(fAA);
        paint.setBlendMode(fMode);
        SkColor color = start_color(fColorType);

        int w = this->getSize().x();
        int h = this->getSize().y();

        static const SkScalar kRectW = 25.1f;
        static const SkScalar kRectH = 25.9f;

        SkMatrix rotate;
        // This value was chosen so that we frequently hit the axis-aligned case.
        rotate.setRotate(30.f, kRectW / 2, kRectH / 2);
        SkMatrix m = rotate;

        SkScalar tx = 0, ty = 0;

        for (int i = 0; i < loops; ++i) {
            canvas->save();
            canvas->translate(tx, ty);
            canvas->concat(m);
            paint.setColor(color);
            color = advance_color(color, fColorType, i);

            canvas->drawRect(SkRect::MakeWH(kRectW, kRectH), paint);
            canvas->restore();

            tx += kRectW + 2;
            if (tx > w) {
                tx = 0;
                ty += kRectH + 2;
                if (ty > h) {
                    ty = 0;
                }
            }

            m.postConcat(rotate);
        }
    }

private:
    void makeName() {
        fName = "rotated_rects";
        if (fAA) {
            fName.append("_aa");
        } else {
            fName.append("_bw");
        }
        switch (fColorType) {
            case kConstantOpaque_ColorType:
                fName.append("_same_opaque");
                break;
            case kConstantTransparent_ColorType:
                fName.append("_same_transparent");
                break;
            case kChangingOpaque_ColorType:
                fName.append("_changing_opaque");
                break;
            case kChangingTransparent_ColorType:
                fName.append("_changing_transparent");
                break;
            case kAlternatingOpaqueAndTransparent_ColorType:
                fName.append("_alternating_transparent_and_opaque");
                break;
        }
        fName.appendf("_%s", to_lower(SkBlendMode_Name(fMode)).c_str());
    }

    bool        fAA;
    ColorType   fColorType;
    SkBlendMode fMode;
    SkString    fName;

    typedef Benchmark INHERITED;
};

// Choose kSrcOver because it always allows coverage and alpha to be conflated. kSrc only allows
// conflation when opaque, and kDarken because it isn't possilbe with standard GL blending.
DEF_BENCH(return new RotRectBench(true,  kConstantOpaque_ColorType,                  SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(true,  kConstantTransparent_ColorType,             SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(true,  kChangingOpaque_ColorType,                  SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(true,  kChangingTransparent_ColorType,             SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(true,  kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kSrcOver);)

DEF_BENCH(return new RotRectBench(false, kConstantOpaque_ColorType,                  SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(false, kConstantTransparent_ColorType,             SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(false, kChangingOpaque_ColorType,                  SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(false, kChangingTransparent_ColorType,             SkBlendMode::kSrcOver);)
DEF_BENCH(return new RotRectBench(false, kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kSrcOver);)

DEF_BENCH(return new RotRectBench(true,  kConstantOpaque_ColorType,                  SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(true,  kConstantTransparent_ColorType,             SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(true,  kChangingOpaque_ColorType,                  SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(true,  kChangingTransparent_ColorType,             SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(true,  kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kSrc);)

DEF_BENCH(return new RotRectBench(false, kConstantOpaque_ColorType,                  SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(false, kConstantTransparent_ColorType,             SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(false, kChangingOpaque_ColorType,                  SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(false, kChangingTransparent_ColorType,             SkBlendMode::kSrc);)
DEF_BENCH(return new RotRectBench(false, kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kSrc);)

DEF_BENCH(return new RotRectBench(true,  kConstantOpaque_ColorType,                  SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(true,  kConstantTransparent_ColorType,             SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(true,  kChangingOpaque_ColorType,                  SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(true,  kChangingTransparent_ColorType,             SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(true,  kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kDarken);)

DEF_BENCH(return new RotRectBench(false, kConstantOpaque_ColorType,                  SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(false, kConstantTransparent_ColorType,             SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(false, kChangingOpaque_ColorType,                  SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(false, kChangingTransparent_ColorType,             SkBlendMode::kDarken);)
DEF_BENCH(return new RotRectBench(false, kAlternatingOpaqueAndTransparent_ColorType, SkBlendMode::kDarken);)
