/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"

// CORE
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkDebugCanvas.h"
#include "SkDocument.h"
#include "SkFontMgr.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkNullCanvas.h"
#include "SkPathEffect.h"
#include "SkPictureRecorder.h"
#include "SkRSXform.h"
#include "SkRegion.h"
#include "SkSurface.h"
#include "SkTypeface.h"

// EFFECTS
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAlphaThresholdFilter.h"
#include "SkArcToPathEffect.h"
#include "SkArithmeticImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkComposeImageFilter.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkGaussianEdgeShader.h"
#include "SkGradientShader.h"
#include "SkHighContrastFilter.h"
#include "SkImageSource.h"
#include "SkLightingImageFilter.h"
#include "SkLumaColorFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPaintImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureImageFilter.h"
#include "SkRRectsGaussianEdgeMaskFilter.h"
#include "SkTableColorFilter.h"
#include "SkTileImageFilter.h"
#include "SkXfermodeImageFilter.h"

// SRC
#include "SkUtils.h"

// MISC

#include <iostream>

// TODO:
//   SkTextBlob with Unicode
//   Cleanup function names
//   Fuzz::nextRangeEnum()

template <typename T, void (SkPaint::*S)(T)>
inline void fuzz_input(Fuzz* fuzz, SkPaint* paint) {
    T value;
    fuzz->next(&value);
    (paint->*S)(value);
}

template <typename T, void (SkPaint::*S)(T)>
inline void fuzz_enum_input(Fuzz* fuzz, SkPaint* paint, T rmin, T rmax) {
    using U = skstd::underlying_type_t<T>;
    U value;
    fuzz->nextRange(&value, (U)rmin, (U)rmax);
    (paint->*S)((T)value);
}

// be careful: `foo(make_bool(f), make_bool(f))` is undefined.
static bool make_bool(Fuzz* fuzz) {
    bool b;
    fuzz->next(&b);
    return b;
}

// We don't always want to test NaNs.
static void fuzz_nice_float(Fuzz* fuzz, float* f) {
    fuzz->next(f);
    if (*f != *f || ::fabs(*f) > 1.0e35f) {
        *f = 0.0f;
    }
}

template <typename... Args>
void fuzz_nice_float(Fuzz* fuzz, float* f, Args... rest) {
    fuzz_nice_float(fuzz, f);
    fuzz_nice_float(fuzz, rest...);
}

static void fuzz_path(Fuzz* fuzz, SkPath* path, int maxOps) {
    if (maxOps < 2) {
        maxOps = 2;
    }
    uint8_t fillType;
    fuzz->nextRange(&fillType, 0, (uint8_t)SkPath::kInverseEvenOdd_FillType);
    path->setFillType((SkPath::FillType)fillType);
    uint8_t numOps;
    fuzz->nextRange(&numOps, 2, maxOps);
    for (uint8_t i = 0; i < numOps; ++i) {
        uint8_t op;
        fuzz->nextRange(&op, 0, 6);
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz_nice_float(fuzz, &a, &b);
                path->moveTo(a, b);
                break;
            case 1:
                fuzz_nice_float(fuzz, &a, &b);
                path->lineTo(a, b);
                break;
            case 2:
                fuzz_nice_float(fuzz, &a, &b, &c, &d);
                path->quadTo(a, b, c, d);
                break;
            case 3:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e, &f);
                path->cubicTo(a, b, c, d, e, f);
                break;
            case 5:
                fuzz_nice_float(fuzz, &a, &b, &c, &d, &e);
                path->arcTo(a, b, c, d, e);
                break;
            case 6:
                path->close();
                break;
            default:
                break;
        }
    }
}

template <>
inline void Fuzz::next(SkRegion* region) {
    uint8_t N;
    this->nextRange(&N, 0, 10);
    for (uint8_t i = 0; i < N; ++i) {
        SkIRect r;
        uint8_t op;
        this->next(&r);
        r.sort();
        this->nextRange(&op, 0, (uint8_t)SkRegion::kLastOp);
        if (!region->op(r, (SkRegion::Op)op)) {
            return;
        }
    }
}

template <>
inline void Fuzz::next(SkShader::TileMode* m) {
    using U = skstd::underlying_type_t<SkShader::TileMode>;
    this->nextRange((U*)m, (U)0, (U)(SkShader::kTileModeCount - 1));
}

template <>
inline void Fuzz::next(SkFilterQuality* q) {
    using U = skstd::underlying_type_t<SkFilterQuality>;
    this->nextRange((U*)q,
                    (U)SkFilterQuality::kNone_SkFilterQuality,
                    (U)SkFilterQuality::kLast_SkFilterQuality);
}

template <>
inline void Fuzz::next(SkMatrix* m) {
    constexpr int kArrayLength = 9;
    SkScalar buffer[kArrayLength];
    int matrixType;
    this->nextRange(&matrixType, 0, 4);
    switch (matrixType) {
        case 0:  // identity
            *m = SkMatrix::I();
            return;
        case 1:  // translate
            this->nextRange(&buffer[0], -4000.0f, 4000.0f);
            this->nextRange(&buffer[1], -4000.0f, 4000.0f);
            *m = SkMatrix::MakeTrans(buffer[0], buffer[1]);
            return;
        case 2:  // translate + scale
            this->nextRange(&buffer[0], -400.0f, 400.0f);
            this->nextRange(&buffer[1], -400.0f, 400.0f);
            this->nextRange(&buffer[2], -4000.0f, 4000.0f);
            this->nextRange(&buffer[3], -4000.0f, 4000.0f);
            *m = SkMatrix::MakeScale(buffer[0], buffer[1]);
            m->postTranslate(buffer[2], buffer[3]);
            return;
        case 3:  // affine
            this->nextN(buffer, 6);
            m->setAffine(buffer);
            return;
        case 4:  // perspective
            this->nextN(buffer, kArrayLength);
            m->set9(buffer);
            return;
        default:
            return;
    }
}

template <>
inline void Fuzz::next(SkRRect* rr) {
    SkRect r;
    SkVector radii[4];
    this->next(&r);
    r.sort();
    for (SkVector& vec : radii) {
        this->nextRange(&vec.fX, 0.0f, 1.0f);
        vec.fX *= 0.5f * r.width();
        this->nextRange(&vec.fY, 0.0f, 1.0f);
        vec.fY *= 0.5f * r.height();
    }
    rr->setRectRadii(r, radii);
}

template <>
inline void Fuzz::next(SkBlendMode* mode) {
    using U = skstd::underlying_type_t<SkBlendMode>;
    this->nextRange((U*)mode, (U)0, (U)SkBlendMode::kLastMode);
}

sk_sp<SkImage> MakeFuzzImage(Fuzz*);

SkBitmap MakeFuzzBitmap(Fuzz*);

static sk_sp<SkPicture> make_picture(Fuzz*, int depth);

sk_sp<SkColorFilter> MakeColorFilter(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    int colorFilterType;
    fuzz->nextRange(&colorFilterType, 0, 8);
    switch (colorFilterType) {
        case 0:
            return nullptr;
        case 1: {
            SkColor color;
            SkBlendMode mode;
            fuzz->next(&color, &mode);
            return SkColorFilter::MakeModeFilter(color, mode);
        }
        case 2: {
            sk_sp<SkColorFilter> outer = MakeColorFilter(fuzz, depth - 1);
            sk_sp<SkColorFilter> inner = MakeColorFilter(fuzz, depth - 1);
            return SkColorFilter::MakeComposeFilter(std::move(outer), std::move(inner));
        }
        case 3: {
            SkScalar array[20];
            fuzz->nextN(array, SK_ARRAY_COUNT(array));
            return SkColorFilter::MakeMatrixFilterRowMajor255(array);
        }
        case 4: {
            SkColor mul, add;
            fuzz->next(&mul, &add);
            return SkColorMatrixFilter::MakeLightingFilter(mul, add);
        }
        case 5: {
            bool grayscale;
            int invertStyle;
            float contrast;
            fuzz->next(&grayscale);
            fuzz->nextRange(&invertStyle, 0, 2);
            fuzz->nextRange(&contrast, -1.0f, 1.0f);
            return SkHighContrastFilter::Make(SkHighContrastConfig(
                    grayscale, SkHighContrastConfig::InvertStyle(invertStyle), contrast));
        }
        case 6:
            return SkLumaColorFilter::Make();
        case 7: {
            uint8_t table[256];
            fuzz->nextN(table, SK_ARRAY_COUNT(table));
            return SkTableColorFilter::Make(table);
        }
        case 8: {
            uint8_t tableA[256];
            uint8_t tableR[256];
            uint8_t tableG[256];
            uint8_t tableB[256];
            fuzz->nextN(tableA, SK_ARRAY_COUNT(tableA));
            fuzz->nextN(tableR, SK_ARRAY_COUNT(tableR));
            fuzz->nextN(tableG, SK_ARRAY_COUNT(tableG));
            fuzz->nextN(tableB, SK_ARRAY_COUNT(tableB));
            return SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB);
        }
    }
    return nullptr;
}

void make_pos(Fuzz* fuzz, SkScalar* pos, int colorCount) {
    SkScalar totalPos = 0;
    for (int i = 0; i < colorCount; ++i) {
        fuzz->nextRange(&pos[i], 1.0f, 1024.0f);
        totalPos += pos[i];
    }
    totalPos = 1.0f / totalPos;
    for (int i = 0; i < colorCount; ++i) {
        pos[i] *= totalPos;
    }
    // SkASSERT(fabs(pos[colorCount - 1] - 1.0f) < 0.00001f);
    pos[colorCount - 1] = 1.0f;
}

sk_sp<SkShader> MakeFuzzShader(Fuzz* fuzz, int depth) {
    sk_sp<SkShader> shader1(nullptr), shader2(nullptr);
    sk_sp<SkColorFilter> colorFilter(nullptr);
    SkBitmap bitmap;
    sk_sp<SkImage> img;
    SkShader::TileMode tmX, tmY;
    bool useMatrix;
    SkColor color;
    SkMatrix matrix;
    SkBlendMode blendMode;
    int shaderType;
    if (depth <= 0) {
        return nullptr;
    }
    fuzz->nextRange(&shaderType, 0, 14);
    switch (shaderType) {
        case 0:
            return nullptr;
        case 1:
            return SkShader::MakeEmptyShader();
        case 2:
            fuzz->next(&color);
            return SkShader::MakeColorShader(color);
        case 3:
            img = MakeFuzzImage(fuzz);
            fuzz->next(&tmX, &tmY, &useMatrix);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            return img->makeShader(tmX, tmY, useMatrix ? &matrix : nullptr);
        case 4:
            bitmap = MakeFuzzBitmap(fuzz);
            fuzz->next(&tmX, &tmY, &useMatrix);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            return SkShader::MakeBitmapShader(bitmap, tmX, tmY, useMatrix ? &matrix : nullptr);
        case 5:
            shader1 = MakeFuzzShader(fuzz, depth - 1);  // limit recursion.
            fuzz->next(&matrix);
            return shader1 ? shader1->makeWithLocalMatrix(matrix) : nullptr;
        case 6:
            shader1 = MakeFuzzShader(fuzz, depth - 1);  // limit recursion.
            colorFilter = MakeColorFilter(fuzz, depth - 1);
            return shader1 ? shader1->makeWithColorFilter(std::move(colorFilter)) : nullptr;
        case 7:
            shader1 = MakeFuzzShader(fuzz, depth - 1);  // limit recursion.
            shader2 = MakeFuzzShader(fuzz, depth - 1);
            fuzz->next(&blendMode);
            return SkShader::MakeComposeShader(std::move(shader1), std::move(shader2), blendMode);
        case 8: {
            auto pic = make_picture(fuzz, depth - 1);
            bool useTile;
            SkRect tile;
            fuzz->next(&tmX, &tmY, &useMatrix, &useTile);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            if (useTile) {
                fuzz->next(&tile);
            }
            return SkShader::MakePictureShader(std::move(pic), tmX, tmY,
                                               useMatrix ? &matrix : nullptr,
                                               useTile ? &tile : nullptr);
        }
        // EFFECTS:
        case 9:
            return SkGaussianEdgeShader::Make();
        case 10: {
            constexpr int kMaxColors = 12;
            SkPoint pts[2];
            SkColor colors[kMaxColors];
            SkScalar pos[kMaxColors];
            int colorCount;
            bool usePos;
            fuzz->nextN(pts, 2);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            fuzz->next(&tmX, &useMatrix, &usePos);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            if (usePos) {
                make_pos(fuzz, pos, colorCount);
            }
            return SkGradientShader::MakeLinear(pts, colors, usePos ? pos : nullptr, colorCount,
                                                tmX, 0, useMatrix ? &matrix : nullptr);
        }
        case 11: {
            constexpr int kMaxColors = 12;
            SkPoint center;
            SkScalar radius;
            int colorCount;
            bool usePos;
            SkColor colors[kMaxColors];
            SkScalar pos[kMaxColors];
            fuzz->next(&tmX, &useMatrix, &usePos, &center, &radius);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            if (usePos) {
                make_pos(fuzz, pos, colorCount);
            }
            return SkGradientShader::MakeRadial(center, radius, colors, usePos ? pos : nullptr,
                                                colorCount, tmX, 0, useMatrix ? &matrix : nullptr);
        }
        case 12: {
            constexpr int kMaxColors = 12;
            SkPoint start, end;
            SkScalar startRadius, endRadius;
            int colorCount;
            bool usePos;
            SkColor colors[kMaxColors];
            SkScalar pos[kMaxColors];
            fuzz->next(&tmX, &useMatrix, &usePos, &startRadius, &endRadius, &start, &end);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            if (usePos) {
                make_pos(fuzz, pos, colorCount);
            }
            return SkGradientShader::MakeTwoPointConical(start, startRadius, end, endRadius, colors,
                                                         usePos ? pos : nullptr, colorCount, tmX, 0,
                                                         useMatrix ? &matrix : nullptr);
        }
        case 13: {
            constexpr int kMaxColors = 12;
            SkScalar cx, cy;
            int colorCount;
            bool usePos;
            SkColor colors[kMaxColors];
            SkScalar pos[kMaxColors];
            fuzz->next(&cx, &cy, &useMatrix, &usePos);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            if (useMatrix) {
                fuzz->next(&matrix);
            }
            if (usePos) {
                make_pos(fuzz, pos, colorCount);
            }
            return SkGradientShader::MakeSweep(cx, cy, colors, usePos ? pos : nullptr, colorCount,
                                               0, useMatrix ? &matrix : nullptr);
        }
        case 14: {
            SkScalar baseFrequencyX, baseFrequencyY, seed;
            int numOctaves;
            SkISize tileSize;
            bool useTileSize, turbulence;
            fuzz->next(&baseFrequencyX, &baseFrequencyY, &seed, &useTileSize, &turbulence);
            if (useTileSize) {
                fuzz->next(&tileSize);
            }
            fuzz->nextRange(&numOctaves, 2, 7);
            if (turbulence) {
                return SkPerlinNoiseShader::MakeTurbulence(baseFrequencyX, baseFrequencyY,
                                                           numOctaves, seed,
                                                           useTileSize ? &tileSize : nullptr);
            } else {
                return SkPerlinNoiseShader::MakeFractalNoise(baseFrequencyX, baseFrequencyY,
                                                             numOctaves, seed,
                                                             useTileSize ? &tileSize : nullptr);
            }
        }
        default:
            break;
    }
    return nullptr;
}

sk_sp<SkPathEffect> MakeFuzzPathEffect(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    uint8_t pathEffectType;
    fuzz->nextRange(&pathEffectType, 0, 9);
    switch (pathEffectType) {
        case 0: {
            return nullptr;
        }
        case 1: {
            sk_sp<SkPathEffect> first = MakeFuzzPathEffect(fuzz, depth - 1);
            sk_sp<SkPathEffect> second = MakeFuzzPathEffect(fuzz, depth - 1);
            return SkPathEffect::MakeSum(std::move(first), std::move(second));
        }
        case 2: {
            sk_sp<SkPathEffect> first = MakeFuzzPathEffect(fuzz, depth - 1);
            sk_sp<SkPathEffect> second = MakeFuzzPathEffect(fuzz, depth - 1);
            return SkPathEffect::MakeCompose(std::move(first), std::move(second));
        }
        case 3: {
            SkPath path;
            fuzz_path(fuzz, &path, 20);
            SkScalar advance, phase;
            fuzz->next(&advance, &phase);
            using U = skstd::underlying_type_t<SkPath1DPathEffect::Style>;
            U style;
            fuzz->nextRange(&style, (U)0, (U)SkPath1DPathEffect::kLastEnum_Style);
            return SkPath1DPathEffect::Make(path, advance, phase, (SkPath1DPathEffect::Style)style);
        }
        case 4: {
            SkScalar width;
            SkMatrix matrix;
            fuzz->next(&width, &matrix);
            return SkLine2DPathEffect::Make(width, matrix);
        }
        case 5: {
            SkPath path;
            fuzz_path(fuzz, &path, 20);
            SkMatrix matrix;
            fuzz->next(&matrix);
            return SkPath2DPathEffect::Make(matrix, path);
        }
        case 6: {
            SkScalar radius;
            fuzz->next(&radius);
            return SkArcToPathEffect::Make(radius);
        }
        case 7: {
            SkScalar radius;
            fuzz->next(&radius);
            return SkCornerPathEffect::Make(radius);
        }
        case 8: {
            SkScalar phase;
            fuzz->next(&phase);
            SkScalar intervals[20];
            int count;
            fuzz->nextRange(&count, 0, (int)SK_ARRAY_COUNT(intervals));
            fuzz->nextN(intervals, count);
            return SkDashPathEffect::Make(intervals, count, phase);
        }
        case 9: {
            SkScalar segLength, dev;
            uint32_t seed;
            fuzz->next(&segLength, &dev, &seed);
            return SkDiscretePathEffect::Make(segLength, dev, seed);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

sk_sp<SkMaskFilter> MakeFuzzMaskFilter(Fuzz* fuzz) {
    int maskfilterType;
    fuzz->nextRange(&maskfilterType, 0, 2);
    switch (maskfilterType) {
        case 0:
            return nullptr;
        case 1: {
            using U = skstd::underlying_type_t<SkBlurStyle>;
            U style;
            fuzz->nextRange(&style, (U)0, (U)kLastEnum_SkBlurStyle);
            SkScalar sigma;
            fuzz->next(&sigma);
            SkRect occluder{0.0f, 0.0f, 0.0f, 0.0f};
            if (make_bool(fuzz)) {
                fuzz->next(&occluder);
            }
            uint32_t flags;
            fuzz->nextRange(&flags, 0, 3);
            return SkBlurMaskFilter::Make((SkBlurStyle)style, sigma, occluder, flags);
        }
        case 2: {
            SkRRect first, second;
            SkScalar radius;
            fuzz->next(&first, &second, &radius);
            return SkRRectsGaussianEdgeMaskFilter::Make(first, second, radius);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

sk_sp<SkTypeface> MakeFuzzTypeface(Fuzz* fuzz) {
    if (make_bool(fuzz)) {
        return nullptr;
    }
    auto fontMugger = SkFontMgr::RefDefault();
    SkASSERT(fontMugger);
    int familyCount = fontMugger->countFamilies();
    int i, j;
    fuzz->nextRange(&i, 0, familyCount - 1);
    sk_sp<SkFontStyleSet> family(fontMugger->createStyleSet(i));
    int styleCount = family->count();
    fuzz->nextRange(&j, 0, styleCount - 1);
    return sk_sp<SkTypeface>(family->createTypeface(j));
}

template <>
inline void Fuzz::next(SkImageFilter::CropRect* cropRect) {
    SkRect rect;
    uint8_t flags;
    this->next(&rect);
    this->nextRange(&flags, 0, 0xF);
    *cropRect = SkImageFilter::CropRect(rect, flags);
}

static sk_sp<SkImageFilter> MakeFuzzImageFilter(Fuzz* fuzz, int depth);

static sk_sp<SkImageFilter> make_fuzz_lighting_imagefilter(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    uint8_t imageFilterType;
    fuzz->nextRange(&imageFilterType, 1, 6);
    SkPoint3 p, q;
    SkColor lightColor;
    SkScalar surfaceScale, k, specularExponent, cutoffAngle, shininess;
    sk_sp<SkImageFilter> input;
    SkImageFilter::CropRect cropRect;
    bool useCropRect;
    fuzz->next(&useCropRect);
    if (useCropRect) {
        fuzz->next(&cropRect);
    }
    switch (imageFilterType) {
        case 1:
            fuzz->next(&p, &lightColor, &surfaceScale, &k);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeDistantLitDiffuse(p, lightColor, surfaceScale, k,
                                                                std::move(input),
                                                                useCropRect ? &cropRect : nullptr);
        case 2:
            fuzz->next(&p, &lightColor, &surfaceScale, &k);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakePointLitDiffuse(p, lightColor, surfaceScale, k,
                                                              std::move(input),
                                                              useCropRect ? &cropRect : nullptr);
        case 3:
            fuzz->next(&p, &q, &specularExponent, &cutoffAngle, &lightColor, &surfaceScale, &k);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeSpotLitDiffuse(
                    p, q, specularExponent, cutoffAngle, lightColor, surfaceScale, k,
                    std::move(input), useCropRect ? &cropRect : nullptr);
        case 4:
            fuzz->next(&p, &lightColor, &surfaceScale, &k, &shininess);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeDistantLitSpecular(p, lightColor, surfaceScale, k,
                                                                 shininess, std::move(input),
                                                                 useCropRect ? &cropRect : nullptr);
        case 5:
            fuzz->next(&p, &lightColor, &surfaceScale, &k, &shininess);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakePointLitSpecular(p, lightColor, surfaceScale, k,
                                                               shininess, std::move(input),
                                                               useCropRect ? &cropRect : nullptr);
        case 6:
            fuzz->next(&p, &q, &specularExponent, &cutoffAngle, &lightColor, &surfaceScale, &k,
                       &shininess);
            input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeSpotLitSpecular(
                    p, q, specularExponent, cutoffAngle, lightColor, surfaceScale, k, shininess,
                    std::move(input), useCropRect ? &cropRect : nullptr);
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static void FuzzPaint(Fuzz* fuzz, SkPaint* paint, int depth);

static sk_sp<SkImageFilter> MakeFuzzImageFilter(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    uint8_t imageFilterType;
    fuzz->nextRange(&imageFilterType, 0, 24);
    switch (imageFilterType) {
        case 0:
            return nullptr;
        case 1: {
            SkScalar sigmaX, sigmaY;
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            SkImageFilter::CropRect cropRect;
            bool useCropRect;
            fuzz->next(&sigmaX, &sigmaY, &useCropRect);
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkImageFilter::MakeBlur(sigmaX, sigmaY, std::move(input),
                                           useCropRect ? &cropRect : nullptr);
        }
        case 2: {
            SkMatrix matrix;
            SkFilterQuality quality;
            fuzz->next(&matrix, &quality);
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkImageFilter::MakeMatrixFilter(matrix, quality, std::move(input));
        }
        case 3: {
            SkRegion region;
            SkScalar innerMin, outerMax;
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            SkImageFilter::CropRect cropRect;
            bool useCropRect;
            fuzz->next(&region, &innerMin, &outerMax, &useCropRect);
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkAlphaThresholdFilter::Make(region, innerMin, outerMax, std::move(input),
                                                useCropRect ? &cropRect : nullptr);
        }
        case 4: {
            float k1, k2, k3, k4;
            bool enforcePMColor;
            bool useCropRect;
            fuzz->next(&k1, &k2, &k3, &k4, &enforcePMColor, &useCropRect);
            sk_sp<SkImageFilter> background = MakeFuzzImageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> foreground = MakeFuzzImageFilter(fuzz, depth - 1);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkArithmeticImageFilter::Make(k1, k2, k3, k4, enforcePMColor,
                                                 std::move(background), std::move(foreground),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 5: {
            sk_sp<SkColorFilter> cf = MakeColorFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            bool useCropRect;
            SkImageFilter::CropRect cropRect;
            fuzz->next(&useCropRect);
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkColorFilterImageFilter::Make(std::move(cf), std::move(input),
                                                  useCropRect ? &cropRect : nullptr);
        }
        case 6: {
            sk_sp<SkImageFilter> ifo = MakeFuzzImageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> ifi = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkComposeImageFilter::Make(std::move(ifo), std::move(ifi));
        }
        case 7: {
            SkDisplacementMapEffect::ChannelSelectorType xChannelSelector, yChannelSelector;
            using U = skstd::underlying_type_t<SkDisplacementMapEffect::ChannelSelectorType>;
            fuzz->nextRange((U*)(&xChannelSelector), 0, 4);
            fuzz->nextRange((U*)(&yChannelSelector), 0, 4);
            SkScalar scale;
            bool useCropRect;
            fuzz->next(&scale, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> displacement = MakeFuzzImageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> color = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                                 std::move(displacement), std::move(color),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 8: {
            SkScalar dx, dy, sigmaX, sigmaY;
            SkColor color;
            SkDropShadowImageFilter::ShadowMode shadowMode;
            using U = skstd::underlying_type_t<SkDropShadowImageFilter::ShadowMode>;
            fuzz->nextRange((U*)(&shadowMode), (U)0, (U)1);
            bool useCropRect;
            fuzz->next(&dx, &dy, &sigmaX, &sigmaY, &color, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color, shadowMode,
                                                 std::move(input),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 9:
            return SkImageSource::Make(MakeFuzzImage(fuzz));
        case 10: {
            sk_sp<SkImage> image = MakeFuzzImage(fuzz);
            SkRect srcRect, dstRect;
            SkFilterQuality filterQuality;
            fuzz->next(&srcRect, &dstRect, &filterQuality);
            return SkImageSource::Make(std::move(image), srcRect, dstRect, filterQuality);
        }
        case 11:
            return make_fuzz_lighting_imagefilter(fuzz, depth - 1);
        case 12: {
            SkRect srcRect;
            SkScalar inset;
            bool useCropRect;
            SkImageFilter::CropRect cropRect;
            fuzz->next(&srcRect, &inset, &useCropRect);
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkMagnifierImageFilter::Make(srcRect, inset, std::move(input),
                                                useCropRect ? &cropRect : nullptr);
        }
        case 13: {
            constexpr int kMaxKernelSize = 5;
            int32_t n, m;
            fuzz->nextRange(&n, 1, kMaxKernelSize);
            fuzz->nextRange(&m, 1, kMaxKernelSize);
            SkScalar kernel[kMaxKernelSize * kMaxKernelSize];
            fuzz->nextN(kernel, n * m);
            int32_t offsetX, offsetY;
            fuzz->nextRange(&offsetX, 0, n - 1);
            fuzz->nextRange(&offsetY, 0, m - 1);
            SkScalar gain, bias;
            bool convolveAlpha, useCropRect;
            fuzz->next(&gain, &bias, &convolveAlpha, &useCropRect);
            SkMatrixConvolutionImageFilter::TileMode tileMode;
            using U = skstd::underlying_type_t<SkMatrixConvolutionImageFilter::TileMode>;
            fuzz->nextRange((U*)(&tileMode), (U)0, (U)2);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkMatrixConvolutionImageFilter::Make(
                    SkISize{n, m}, kernel, gain, bias, SkIPoint{offsetX, offsetY}, tileMode,
                    convolveAlpha, std::move(input), useCropRect ? &cropRect : nullptr);
        }
        case 14: {
            sk_sp<SkImageFilter> first = MakeFuzzImageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> second = MakeFuzzImageFilter(fuzz, depth - 1);
            SkBlendMode blendMode;
            bool useCropRect;
            fuzz->next(&useCropRect, &blendMode);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkMergeImageFilter::Make(std::move(first), std::move(second), blendMode,
                                            useCropRect ? &cropRect : nullptr);
        }
        case 15: {
            constexpr int kMaxCount = 4;
            sk_sp<SkImageFilter> ifs[kMaxCount];
            SkBlendMode blendModes[kMaxCount];
            int count;
            fuzz->nextRange(&count, 1, kMaxCount);
            for (int i = 0; i < count; ++i) {
                ifs[i] = MakeFuzzImageFilter(fuzz, depth - 1);
            }
            fuzz->nextN(blendModes, count);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkMergeImageFilter::MakeN(ifs, count, blendModes,
                                             useCropRect ? &cropRect : nullptr);
        }
        case 16: {
            int rx, ry;
            fuzz->next(&rx, &ry);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkDilateImageFilter::Make(rx, ry, std::move(input),
                                             useCropRect ? &cropRect : nullptr);
        }
        case 17: {
            int rx, ry;
            fuzz->next(&rx, &ry);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkErodeImageFilter::Make(rx, ry, std::move(input),
                                            useCropRect ? &cropRect : nullptr);
        }
        case 18: {
            SkScalar dx, dy;
            fuzz->next(&dx, &dy);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkOffsetImageFilter::Make(dx, dy, std::move(input),
                                             useCropRect ? &cropRect : nullptr);
        }
        case 19: {
            SkPaint paint;
            FuzzPaint(fuzz, &paint, depth - 1);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            return SkPaintImageFilter::Make(paint, useCropRect ? &cropRect : nullptr);
        }
        case 20: {
            sk_sp<SkPicture> picture = make_picture(fuzz, depth - 1);
            return SkPictureImageFilter::Make(std::move(picture));
        }
        case 21: {
            SkRect cropRect;
            fuzz->next(&cropRect);
            sk_sp<SkPicture> picture = make_picture(fuzz, depth - 1);
            return SkPictureImageFilter::Make(std::move(picture), cropRect);
        }
        case 22: {
            SkRect cropRect;
            SkFilterQuality filterQuality;
            fuzz->next(&cropRect, &filterQuality);
            sk_sp<SkPicture> picture = make_picture(fuzz, depth - 1);
            return SkPictureImageFilter::MakeForLocalSpace(std::move(picture), cropRect,
                                                           filterQuality);
        }
        case 23: {
            SkRect src, dst;
            fuzz->next(&src, &dst);
            sk_sp<SkImageFilter> input = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkTileImageFilter::Make(src, dst, std::move(input));
        }
        case 24: {
            SkBlendMode blendMode;
            bool useCropRect;
            fuzz->next(&useCropRect, &blendMode);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&useCropRect);
            }
            sk_sp<SkImageFilter> bg = MakeFuzzImageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> fg = MakeFuzzImageFilter(fuzz, depth - 1);
            return SkXfermodeImageFilter::Make(blendMode, std::move(bg), std::move(fg),
                                               useCropRect ? &cropRect : nullptr);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

sk_sp<SkImage> MakeFuzzImage(Fuzz* fuzz) {
    int w, h;
    fuzz->nextRange(&w, 1, 1024);
    fuzz->nextRange(&h, 1, 1024);
    SkAutoTMalloc<SkPMColor> data(w * h);
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(w, h), data.get(), w * sizeof(SkPMColor));
    int n = w * h;
    for (int i = 0; i < n; ++i) {
        SkColor c;
        fuzz->next(&c);
        data[i] = SkPreMultiplyColor(c);
    }
    (void)data.release();
    return SkImage::MakeFromRaster(pixmap, [](const void* p, void*) { sk_free((void*)p); },
                                   nullptr);
}

SkBitmap MakeFuzzBitmap(Fuzz* fuzz) {
    SkBitmap bitmap;
    int w, h;
    fuzz->nextRange(&w, 1, 1024);
    fuzz->nextRange(&h, 1, 1024);
    bitmap.allocN32Pixels(w, h);
    SkAutoLockPixels autoLockPixels(bitmap);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            SkColor c;
            fuzz->next(&c);
            *bitmap.getAddr32(x, y) = SkPreMultiplyColor(c);
        }
    }
    return bitmap;
}

void FuzzPaint(Fuzz* fuzz, SkPaint* paint, int depth) {
    if (!fuzz || !paint || depth <= 0) {
        return;
    }

    fuzz_input<bool, &SkPaint::setAntiAlias>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setDither>(fuzz, paint);
    fuzz_input<SkColor, &SkPaint::setColor>(fuzz, paint);
    fuzz_enum_input<SkBlendMode, &SkPaint::setBlendMode>(fuzz, paint, (SkBlendMode)0,
                                                         SkBlendMode::kLastMode);
    fuzz_enum_input<SkFilterQuality, &SkPaint::setFilterQuality>(
            fuzz, paint, SkFilterQuality::kNone_SkFilterQuality,
            SkFilterQuality::kLast_SkFilterQuality);
    fuzz_enum_input<SkPaint::Style, &SkPaint::setStyle>(fuzz, paint, SkPaint::kFill_Style,
                                                        SkPaint::kStrokeAndFill_Style);
    paint->setShader(MakeFuzzShader(fuzz, depth - 1));
    paint->setPathEffect(MakeFuzzPathEffect(fuzz, depth - 1));
    paint->setMaskFilter(MakeFuzzMaskFilter(fuzz));
    paint->setImageFilter(MakeFuzzImageFilter(fuzz, depth - 1));
    paint->setColorFilter(MakeColorFilter(fuzz, depth - 1));

    if (paint->getStyle() != SkPaint::kFill_Style) {
        fuzz_input<SkScalar, &SkPaint::setStrokeWidth>(fuzz, paint);
        fuzz_input<SkScalar, &SkPaint::setStrokeMiter>(fuzz, paint);
        fuzz_enum_input<SkPaint::Cap, &SkPaint::setStrokeCap>(fuzz, paint, SkPaint::kButt_Cap,
                                                              SkPaint::kLast_Cap);
        fuzz_enum_input<SkPaint::Join, &SkPaint::setStrokeJoin>(fuzz, paint, SkPaint::kMiter_Join,
                                                                SkPaint::kLast_Join);
    }
}

void FuzzPaintText(Fuzz* fuzz, SkPaint* paint) {
    paint->setTypeface(MakeFuzzTypeface(fuzz));
    fuzz_input<SkScalar, &SkPaint::setTextSize>(fuzz, paint);
    fuzz_input<SkScalar, &SkPaint::setTextScaleX>(fuzz, paint);
    fuzz_input<SkScalar, &SkPaint::setTextSkewX>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setLinearText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setSubpixelText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setLCDRenderText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setEmbeddedBitmapText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setAutohinted>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setVerticalText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setFakeBoldText>(fuzz, paint);
    fuzz_input<bool, &SkPaint::setDevKernText>(fuzz, paint);
    fuzz_enum_input<SkPaint::Hinting, &SkPaint::setHinting>(fuzz, paint, SkPaint::kNo_Hinting,
                                                            SkPaint::kFull_Hinting);
    fuzz_enum_input<SkPaint::Align, &SkPaint::setTextAlign>(fuzz, paint, SkPaint::kLeft_Align,
                                                            SkPaint::kRight_Align);
}

static void fuzz_paint_text_encoding(Fuzz* fuzz, SkPaint* paint) {
    fuzz_enum_input<SkPaint::TextEncoding, &SkPaint::setTextEncoding>(
            fuzz, paint, SkPaint::kUTF8_TextEncoding, SkPaint::kGlyphID_TextEncoding);
}

constexpr int kMaxGlyphCount = 30;

SkTDArray<uint8_t> make_fuzz_text(Fuzz* fuzz, const SkPaint& paint) {
    SkTDArray<uint8_t> array;
    if (SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding()) {
        int glyphRange = paint.getTypeface() ? paint.getTypeface()->countGlyphs()
                                             : SkTypeface::MakeDefault()->countGlyphs();
        int glyphCount;
        fuzz->nextRange(&glyphCount, 1, kMaxGlyphCount);
        SkGlyphID* glyphs = (SkGlyphID*)array.append(glyphCount * sizeof(SkGlyphID));
        for (int i = 0; i < glyphCount; ++i) {
            fuzz->nextRange(&glyphs[i], 0, glyphRange - 1);
        }
        return array;
    }
    static const SkUnichar ranges[][2] = {
        {0x0020, 0x007F},
        {0x00A1, 0x0250},
        {0x0400, 0x0500},
    };
    int32_t count = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(ranges); ++i) {
        count += (ranges[i][1] - ranges[i][0]);
    }
    constexpr int kMaxLength = kMaxGlyphCount;
    SkUnichar buffer[kMaxLength];
    int length;
    fuzz->nextRange(&length, 1, kMaxLength);
    for (int j = 0; j < length; ++j) {
        int32_t value;
        fuzz->nextRange(&value, 0, count - 1);
        for (size_t i = 0; i < SK_ARRAY_COUNT(ranges); ++i) {
            if (value + ranges[i][0] < ranges[i][1]) {
                buffer[j] = value + ranges[i][0];
                break;
            } else {
                value -= (ranges[i][1] - ranges[i][0]);
            }
        }
    }
    switch (paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding: {
            size_t utf8len = 0;
            for (int j = 0; j < length; ++j) {
                utf8len += SkUTF8_FromUnichar(buffer[j], nullptr);
            }
            char* ptr = (char*)array.append(utf8len);
            for (int j = 0; j < length; ++j) {
                ptr += SkUTF8_FromUnichar(buffer[j], ptr);
            }
        } break;
        case SkPaint::kUTF16_TextEncoding: {
            size_t utf16len = 0;
            for (int j = 0; j < length; ++j) {
                utf16len += SkUTF16_FromUnichar(buffer[j]);
            }
            uint16_t* ptr = (uint16_t*)array.append(utf16len * sizeof(uint16_t));
            for (int j = 0; j < length; ++j) {
                ptr += SkUTF16_FromUnichar(buffer[j], ptr);
            }
        } break;
        case SkPaint::kUTF32_TextEncoding:
            memcpy(array.append(length * sizeof(SkUnichar)), buffer, length * sizeof(SkUnichar));
            break;
        default:
            SkASSERT(false);
    }
    return array;
}

static sk_sp<SkTextBlob> make_fuzz_textblob(Fuzz* fuzz) {
    SkTextBlobBuilder textBlobBuilder;
    int8_t runCount;
    fuzz->nextRange(&runCount, (int8_t)1, (int8_t)8);
    while (runCount-- > 0) {
        SkPaint paint;
        fuzz_paint_text_encoding(fuzz, &paint);
        fuzz_input<bool, &SkPaint::setAntiAlias>(fuzz, &paint);
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
        int glyphCount = paint.countText(text.begin(), SkToSizeT(text.count()));
        SkASSERT(glyphCount <= kMaxGlyphCount);
        SkScalar x, y;
        const SkTextBlobBuilder::RunBuffer* buffer;
        uint8_t runType;
        fuzz->nextRange(&runType, (uint8_t)0, (uint8_t)2);
        switch (runType) {
            case 0:
                fuzz->next(&x, &y);
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRun(paint, glyphCount, x, y);
                memcpy(buffer->glyphs, text.begin(), SkToSizeT(text.count()));
                break;
            case 1:
                fuzz->next(&y);
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRunPosH(paint, glyphCount, y);
                memcpy(buffer->glyphs, text.begin(), SkToSizeT(text.count()));
                fuzz->nextN(buffer->pos, glyphCount);
                break;
            case 2:
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRunPos(paint, glyphCount);
                memcpy(buffer->glyphs, text.begin(), SkToSizeT(text.count()));
                fuzz->nextN(buffer->pos, glyphCount * 2);
                break;
            default:
                SkASSERT(false);
        }
    }
    return textBlobBuilder.make();
}

void fuzz_canvas(Fuzz* fuzz, SkCanvas* canvas, int depth = 9) {
    if (!fuzz || !canvas || depth <= 0) {
        return;
    }
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    unsigned N;
    fuzz->nextRange(&N, 0, 2000);
    for (unsigned i = 0; i < N; ++i) {
        if (fuzz->exhausted()) {
            return;
        }
        SkPaint paint;
        SkMatrix matrix;
        unsigned drawCommand;
        fuzz->nextRange(&drawCommand, 0, 54);
        switch (drawCommand) {
            case 0:
                canvas->flush();
                break;
            case 1:
                canvas->save();
                break;
            case 2: {
                SkRect bounds;
                fuzz->next(&bounds);
                FuzzPaint(fuzz, &paint, depth - 1);
                canvas->saveLayer(&bounds, &paint);
                break;
            }
            case 3: {
                SkRect bounds;
                fuzz->next(&bounds);
                canvas->saveLayer(&bounds, nullptr);
                break;
            }
            case 4:
                FuzzPaint(fuzz, &paint, depth - 1);
                canvas->saveLayer(nullptr, &paint);
                break;
            case 5:
                canvas->saveLayer(nullptr, nullptr);
                break;
            case 6: {
                uint8_t alpha;
                fuzz->next(&alpha);
                canvas->saveLayerAlpha(nullptr, (U8CPU)alpha);
                break;
            }
            case 7: {
                SkRect bounds;
                uint8_t alpha;
                fuzz->next(&bounds, &alpha);
                canvas->saveLayerAlpha(&bounds, (U8CPU)alpha);
                break;
            }
            case 8: {
                SkCanvas::SaveLayerRec saveLayerRec;
                SkRect bounds;
                if (make_bool(fuzz)) {
                    fuzz->next(&bounds);
                    saveLayerRec.fBounds = &bounds;
                }
                if (make_bool(fuzz)) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                    saveLayerRec.fPaint = &paint;
                }
                sk_sp<SkImageFilter> imageFilter;
                if (make_bool(fuzz)) {
                    imageFilter = MakeFuzzImageFilter(fuzz, depth - 1);
                    saveLayerRec.fBackdrop = imageFilter.get();
                }
                // _DumpCanvas can't handle this.
                // if (make_bool(fuzz)) {
                //     saveLayerRec.fSaveLayerFlags |= SkCanvas::kIsOpaque_SaveLayerFlag;
                // }
                // if (make_bool(fuzz)) {
                //     saveLayerRec.fSaveLayerFlags |= SkCanvas::kPreserveLCDText_SaveLayerFlag;
                // }

                canvas->saveLayer(saveLayerRec);
                break;
            }
            case 9:
                canvas->restore();
                break;
            case 10: {
                int saveCount;
                fuzz->next(&saveCount);
                canvas->restoreToCount(saveCount);
                break;
            }
            case 11: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->translate(x, y);
                break;
            }
            case 12: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->scale(x, y);
                break;
            }
            case 13: {
                SkScalar v;
                fuzz->next(&v);
                canvas->rotate(v);
                break;
            }
            case 14: {
                SkScalar x, y, v;
                fuzz->next(&x, &y, &v);
                canvas->rotate(v, x, y);
                break;
            }
            case 15: {
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->skew(x, y);
                break;
            }
            case 16: {
                SkMatrix mat;
                fuzz->next(&mat);
                canvas->concat(mat);
                break;
            }
            case 17: {
                SkMatrix mat;
                fuzz->next(&mat);
                canvas->setMatrix(mat);
                break;
            }
            case 18:
                canvas->resetMatrix();
                break;
            case 19: {
                SkRect r;
                int op;
                bool doAntiAlias;
                fuzz->next(&r, &doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                r.sort();
                canvas->clipRect(r, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 20: {
                SkRRect rr;
                int op;
                bool doAntiAlias;
                fuzz->next(&rr);
                fuzz->next(&doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRRect(rr, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 21: {
                SkPath path;
                fuzz_path(fuzz, &path, 30);
                int op;
                bool doAntiAlias;
                fuzz->next(&doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipPath(path, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 22: {
                SkRegion region;
                int op;
                fuzz->next(&region);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRegion(region, (SkClipOp)op);
                break;
            }
            case 23:
                FuzzPaint(fuzz, &paint, depth - 1);
                canvas->drawPaint(paint);
                break;
            case 24: {
                FuzzPaint(fuzz, &paint, depth - 1);
                uint8_t pointMode;
                fuzz->nextRange(&pointMode, 0, 3);
                size_t count;
                constexpr int kMaxCount = 30;
                fuzz->nextRange(&count, 0, kMaxCount);
                SkPoint pts[kMaxCount];
                fuzz->nextN(pts, count);
                canvas->drawPoints((SkCanvas::PointMode)pointMode, count, pts, paint);
                break;
            }
            case 25: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRect r;
                fuzz->next(&r);
                canvas->drawRect(r, paint);
                break;
            }
            case 26: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRegion region;
                fuzz->next(&region);
                canvas->drawRegion(region, paint);
                break;
            }
            case 27: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRect r;
                fuzz->next(&r);
                canvas->drawOval(r, paint);
                break;
            }
            case 29: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRRect rr;
                fuzz->next(&rr);
                canvas->drawRRect(rr, paint);
                break;
            }
            case 30: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRRect orr, irr;
                fuzz->next(&orr);
                fuzz->next(&irr);
                if (orr.getBounds().contains(irr.getBounds())) {
                    canvas->drawDRRect(orr, irr, paint);
                }
                break;
            }
            case 31: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkRect r;
                SkScalar start, sweep;
                bool useCenter;
                fuzz->next(&r, &start, &sweep, &useCenter);
                canvas->drawArc(r, start, sweep, useCenter, paint);
                break;
            }
            case 32: {
                SkPath path;
                fuzz_path(fuzz, &path, 60);
                canvas->drawPath(path, paint);
                break;
            }
            case 33: {
                sk_sp<SkImage> img = MakeFuzzImage(fuzz);
                SkScalar left, top;
                bool usePaint;
                fuzz->next(&left, &top, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                canvas->drawImage(img.get(), left, top, usePaint ? &paint : nullptr);
                break;
            }
            case 34: {
                auto img = MakeFuzzImage(fuzz);
                SkRect src, dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawImageRect(img, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 35: {
                auto img = MakeFuzzImage(fuzz);
                SkIRect src;
                SkRect dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawImageRect(img, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 36: {
                bool usePaint;
                auto img = MakeFuzzImage(fuzz);
                SkRect dst;
                fuzz->next(&dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawImageRect(img, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 37: {
                auto img = MakeFuzzImage(fuzz);
                SkIRect center;
                SkRect dst;
                bool usePaint;
                fuzz->next(&center, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                canvas->drawImageNine(img, center, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 38: {
                SkBitmap bitmap = MakeFuzzBitmap(fuzz);
                SkScalar left, top;
                bool usePaint;
                fuzz->next(&left, &top, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                canvas->drawBitmap(bitmap, left, top, usePaint ? &paint : nullptr);
                break;
            }
            case 39: {
                SkBitmap bitmap = MakeFuzzBitmap(fuzz);
                SkRect src, dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(bitmap, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 40: {
                SkBitmap img = MakeFuzzBitmap(fuzz);
                SkIRect src;
                SkRect dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(img, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 41: {
                SkBitmap img = MakeFuzzBitmap(fuzz);
                SkRect dst;
                bool usePaint;
                fuzz->next(&dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_bool(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                        : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(img, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 42: {
                SkBitmap img = MakeFuzzBitmap(fuzz);
                SkIRect center;
                SkRect dst;
                bool usePaint;
                fuzz->next(&center, &dst, &usePaint);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                canvas->drawBitmapNine(img, center, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 43: {
                SkBitmap img = MakeFuzzBitmap(fuzz);
                bool usePaint;
                SkRect dst;
                fuzz->next(&usePaint, &dst);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                constexpr int kMax = 6;
                int xDivs[kMax], yDivs[kMax];
                SkCanvas::Lattice lattice{xDivs, yDivs, nullptr, 0, 0, nullptr};
                fuzz->nextRange(&lattice.fXCount, 2, kMax);
                fuzz->nextRange(&lattice.fYCount, 2, kMax);
                fuzz->nextN(xDivs, lattice.fXCount);
                fuzz->nextN(yDivs, lattice.fYCount);
                canvas->drawBitmapLattice(img, lattice, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 44: {
                auto img = MakeFuzzImage(fuzz);
                bool usePaint;
                SkRect dst;
                fuzz->next(&usePaint, &dst);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                constexpr int kMax = 6;
                int xDivs[kMax], yDivs[kMax];
                SkCanvas::Lattice lattice{xDivs, yDivs, nullptr, 0, 0, nullptr};
                fuzz->nextRange(&lattice.fXCount, 2, kMax);
                fuzz->nextRange(&lattice.fYCount, 2, kMax);
                fuzz->nextN(xDivs, lattice.fXCount);
                fuzz->nextN(yDivs, lattice.fYCount);
                canvas->drawImageLattice(img.get(), lattice, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 45: {
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkScalar x, y;
                fuzz->next(&x, &y);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                canvas->drawText(text.begin(), SkToSizeT(text.count()), x, y, paint);
                break;
            }
            case 46: {
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                int glyphCount = paint.countText(text.begin(), SkToSizeT(text.count()));
                if (glyphCount < 1) {
                    break;
                }
                SkAutoTMalloc<SkPoint> pos(glyphCount);
                SkAutoTMalloc<SkScalar> widths(glyphCount);
                paint.getTextWidths(text.begin(), SkToSizeT(text.count()), widths.get());
                pos[0] = {0, 0};
                for (int i = 1; i < glyphCount; ++i) {
                    float y;
                    fuzz->nextRange(&y, -0.5f * paint.getTextSize(), 0.5f * paint.getTextSize());
                    pos[i] = {pos[i - 1].x() + widths[i - 1], y};
                }
                canvas->drawPosText(text.begin(), SkToSizeT(text.count()), pos.get(), paint);
                break;
            }
            case 47: {
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                int glyphCount = paint.countText(text.begin(), SkToSizeT(text.count()));
                SkAutoTMalloc<SkScalar> widths(glyphCount);
                if (glyphCount < 1) {
                    break;
                }
                paint.getTextWidths(text.begin(), SkToSizeT(text.count()), widths.get());
                SkScalar x = widths[0];
                for (int i = 0; i < glyphCount; ++i) {
                    SkTSwap(x, widths[i]);
                    x += widths[i];
                    SkScalar offset;
                    fuzz->nextRange(&offset, -0.125f * paint.getTextSize(),
                                    0.125f * paint.getTextSize());
                    widths[i] += offset;
                }
                SkScalar y;
                fuzz->next(&y);
                canvas->drawPosTextH(text.begin(), SkToSizeT(text.count()), widths.get(), y, paint);
                break;
            }
            case 48: {
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                SkPath path;
                fuzz_path(fuzz, &path, 20);
                SkScalar hOffset, vOffset;
                fuzz->next(&hOffset, &vOffset);
                canvas->drawTextOnPathHV(text.begin(), SkToSizeT(text.count()), path, hOffset,
                                         vOffset, paint);
                break;
            }
            case 49: {
                SkMatrix matrix;
                bool useMatrix = make_bool(fuzz);
                if (useMatrix) {
                    fuzz->next(&matrix);
                }
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                SkPath path;
                fuzz_path(fuzz, &path, 20);
                canvas->drawTextOnPath(text.begin(), SkToSizeT(text.count()), path,
                                       useMatrix ? &matrix : nullptr, paint);
                break;
            }
            case 50: {
                FuzzPaint(fuzz, &paint, depth - 1);
                FuzzPaintText(fuzz, &paint);
                fuzz_paint_text_encoding(fuzz, &paint);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, paint);
                SkRSXform rSXform[kMaxGlyphCount];
                int glyphCount = paint.countText(text.begin(), SkToSizeT(text.count()));
                SkASSERT(glyphCount <= kMaxGlyphCount);
                fuzz->nextN(rSXform, glyphCount);
                SkRect cullRect;
                bool useCullRect;
                fuzz->next(&useCullRect);
                if (useCullRect) {
                    fuzz->next(&cullRect);
                }
                canvas->drawTextRSXform(text.begin(), SkToSizeT(text.count()), rSXform,
                                        useCullRect ? &cullRect : nullptr, paint);
                break;
            }
            case 51: {
                sk_sp<SkTextBlob> blob = make_fuzz_textblob(fuzz);
                FuzzPaint(fuzz, &paint, depth - 1);
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->drawTextBlob(blob, x, y, paint);
                break;
            }
            case 52: {
                bool usePaint, useMatrix;
                fuzz->next(&usePaint, &useMatrix);
                if (usePaint) {
                    FuzzPaint(fuzz, &paint, depth - 1);
                }
                if (useMatrix) {
                    fuzz->next(&matrix);
                }
                auto pic = make_picture(fuzz, depth - 1);
                canvas->drawPicture(pic, useMatrix ? &matrix : nullptr,
                                    usePaint ? &paint : nullptr);
                break;
            }
            case 53: {
                FuzzPaint(fuzz, &paint, depth - 1);
                SkCanvas::VertexMode vertexMode;
                SkBlendMode mode;
                uint8_t vm, bm;
                fuzz->nextRange(&vm, 0, (uint8_t)SkCanvas::kTriangleFan_VertexMode);
                fuzz->nextRange(&bm, 0, (uint8_t)SkBlendMode::kLastMode);
                vertexMode = (SkCanvas::VertexMode)vm;
                mode = (SkBlendMode)bm;
                constexpr int kMaxCount = 100;
                int vertexCount;
                SkPoint vertices[kMaxCount];
                SkPoint texs[kMaxCount];
                SkColor colors[kMaxCount];
                fuzz->nextRange(&vertexCount, 3, kMaxCount);
                fuzz->nextN(vertices, vertexCount);
                bool useTexs, useColors;
                fuzz->next(&useTexs, &useColors);
                if (useTexs) {
                    fuzz->nextN(texs, vertexCount);
                }
                if (useColors) {
                    fuzz->nextN(colors, vertexCount);
                }
                int indexCount = 0;
                uint16_t indices[kMaxCount * 2];
                if (make_bool(fuzz)) {
                    fuzz->nextRange(&indexCount, vertexCount, vertexCount + kMaxCount);
                    for (int i = 0; i < indexCount; ++i) {
                        fuzz->nextRange(&indices[i], 0, vertexCount - 1);
                    }
                }
                canvas->drawVertices(vertexMode, vertexCount, vertices, useTexs ? texs : nullptr,
                                     useColors ? colors : nullptr, mode,
                                     indexCount > 0 ? indices : nullptr, indexCount, paint);
                break;
            }
            case 54: {
                // canvas->drawVertices(...);
                // TODO
                break;
            }
            default:
                break;
        }
    }
}

static sk_sp<SkPicture> make_picture(Fuzz* fuzz, int depth) {
    SkScalar w, h;
    fuzz->next(&w, &h);
    SkPictureRecorder pictureRecorder;
    fuzz_canvas(fuzz, pictureRecorder.beginRecording(w, h), depth - 1);
    return pictureRecorder.finishRecordingAsPicture();
}

DEF_FUZZ(NullCanvas, fuzz) {
    fuzz_canvas(fuzz, SkMakeNullCanvas().get());
}

DEF_FUZZ(RasterN32Canvas, fuzz) {
    fuzz_canvas(fuzz, SkMakeNullCanvas().get());
    auto surface = SkSurface::MakeRasterN32Premul(612, 792);
    SkASSERT(surface && surface->getCanvas());
    fuzz_canvas(fuzz, surface->getCanvas());
}

DEF_FUZZ(PDFCanvas, fuzz) {
    struct final : public SkWStream {
        bool write(const void*, size_t n) override { fN += n; return true; }
        size_t bytesWritten() const override { return fN; }
        size_t fN = 0;
    } stream;
    auto doc = SkDocument::MakePDF(&stream);
    fuzz_canvas(fuzz, doc->beginPage(612.0f, 792.0f));
}

// not a "real" thing to fuzz, used to debug errors found while fuzzing.
DEF_FUZZ(_DumpCanvas, fuzz) {
    SkDebugCanvas debugCanvas(612, 792);
    fuzz_canvas(fuzz, &debugCanvas);
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    Json::Value json = debugCanvas.toJSON(dataManager, debugCanvas.getSize(), nullCanvas.get());
    Json::StyledStreamWriter("  ").write(std::cout, json);
}
