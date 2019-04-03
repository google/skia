/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlendModePriv.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
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
    kShaderOpaque_ColorType
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
        case kShaderOpaque_ColorType:
            return SK_ColorWHITE;
    }
    SK_ABORT("Shouldn't reach here.");
    return 0;
}

static inline SkColor advance_color(SkColor old, ColorType ct, int step) {
    if (kAlternatingOpaqueAndTransparent_ColorType == ct) {
        ct = (step & 0x1) ? kChangingOpaque_ColorType : kChangingTransparent_ColorType ;
    }
    switch (ct) {
        case kConstantOpaque_ColorType:
        case kConstantTransparent_ColorType:
        case kShaderOpaque_ColorType:
            return old;
        case kChangingOpaque_ColorType:
            return 0xFF000000 | (old + 0x00010307);
        case kChangingTransparent_ColorType:
            return (0x00FFFFFF & (old + 0x00010307)) | 0x80000000;
        case kAlternatingOpaqueAndTransparent_ColorType:
            SK_ABORT("Can't get here");
    }
    SK_ABORT("Shouldn't reach here.");
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
    RotRectBench(bool aa, ColorType ct, SkBlendMode mode, bool perspective = false)
        : fAA(aa)
        , fPerspective(perspective)
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

        if (fColorType == kShaderOpaque_ColorType) {
            // The only requirement for the shader is that it requires local coordinates
            SkPoint pts[2] = { {0.0f, 0.0f}, {kRectW, kRectH} };
            SkColor colors[] = { color, SK_ColorBLUE };
            paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                         SkTileMode::kClamp));
        }

        SkMatrix rotate;
        // This value was chosen so that we frequently hit the axis-aligned case.
        rotate.setRotate(30.f, kRectW / 2, kRectH / 2);
        SkMatrix m = rotate;

        SkScalar tx = 0, ty = 0;

        if (fPerspective) {
            // Apply some fixed perspective to change how ops may draw the rects
            SkMatrix perspective;
            perspective.setIdentity();
            perspective.setPerspX(1e-4f);
            perspective.setPerspY(1e-3f);
            perspective.setSkewX(0.1f);
            canvas->concat(perspective);
        }

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
        if (fPerspective) {
            fName.append("_persp");
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
            case kShaderOpaque_ColorType:
                fName.append("_shader_opaque");
                break;
        }
        fName.appendf("_%s", to_lower(SkBlendMode_Name(fMode)).c_str());
    }

    bool        fAA;
    bool        fPerspective;
    ColorType   fColorType;
    SkBlendMode fMode;
    SkString    fName;

    typedef Benchmark INHERITED;
};

#define DEF_FOR_COLOR_TYPES(aa, blend) \
    DEF_BENCH(return new RotRectBench(aa,  kConstantOpaque_ColorType,                  blend);) \
    DEF_BENCH(return new RotRectBench(aa,  kConstantTransparent_ColorType,             blend);) \
    DEF_BENCH(return new RotRectBench(aa,  kChangingOpaque_ColorType,                  blend);) \
    DEF_BENCH(return new RotRectBench(aa,  kChangingTransparent_ColorType,             blend);) \
    DEF_BENCH(return new RotRectBench(aa,  kAlternatingOpaqueAndTransparent_ColorType, blend);) \
    DEF_BENCH(return new RotRectBench(aa,  kShaderOpaque_ColorType,                    blend);)
#define DEF_FOR_AA_MODES(blend) \
    DEF_FOR_COLOR_TYPES(true, blend) \
    DEF_FOR_COLOR_TYPES(false, blend)

// Choose kSrcOver because it always allows coverage and alpha to be conflated. kSrc only allows
// conflation when opaque, and kDarken because it isn't possilbe with standard GL blending.
DEF_FOR_AA_MODES(SkBlendMode::kSrcOver)
DEF_FOR_AA_MODES(SkBlendMode::kSrc)
DEF_FOR_AA_MODES(SkBlendMode::kDarken)

// Only do a limited run of perspective tests
#define DEF_FOR_PERSP_MODES(aa) \
    DEF_BENCH(return new RotRectBench(aa, kConstantOpaque_ColorType, SkBlendMode::kSrcOver, true);)\
    DEF_BENCH(return new RotRectBench(aa, kShaderOpaque_ColorType, SkBlendMode::kSrcOver, true);)
DEF_FOR_PERSP_MODES(true)
DEF_FOR_PERSP_MODES(false)
