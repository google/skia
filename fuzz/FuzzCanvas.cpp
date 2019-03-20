/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"

// CORE
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkDebugCanvas.h"
#include "SkFontMgr.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkNullCanvas.h"
#include "SkOSFile.h"
#include "SkPDFDocument.h"
#include "SkPathEffect.h"
#include "SkPicturePriv.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkRSXform.h"
#include "SkRegion.h"
#include "SkSurface.h"
#include "SkTo.h"
#include "SkTypeface.h"

// EFFECTS
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAlphaThresholdFilter.h"
#include "SkArithmeticImageFilter.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkComposeImageFilter.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
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
#include "SkReadBuffer.h"
#include "SkTableColorFilter.h"
#include "SkTextBlob.h"
#include "SkTileImageFilter.h"
#include "SkXfermodeImageFilter.h"

// SRC
#include "CommandLineFlags.h"
#include "SkUTF.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLUtil.h"
#endif

// MISC

#include <iostream>
#include <utility>

DEFINE_bool2(gpuInfo, g, false, "Display GPU information on relevant targets.");

// TODO:
//   SkTextBlob with Unicode
//   SkImage: more types

// be careful: `foo(make_fuzz_t<T>(f), make_fuzz_t<U>(f))` is undefined.
// In fact, all make_fuzz_foo() functions have this potential problem.
// Use sequence points!
template <typename T>
inline T make_fuzz_t(Fuzz* fuzz) {
    T t;
    fuzz->next(&t);
    return t;
}

static sk_sp<SkImage> make_fuzz_image(Fuzz*);

static SkBitmap make_fuzz_bitmap(Fuzz*);

static sk_sp<SkPicture> make_fuzz_picture(Fuzz*, int depth);

static sk_sp<SkColorFilter> make_fuzz_colorfilter(Fuzz* fuzz, int depth) {
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
            fuzz->next(&color);
            fuzz->nextRange(&mode, 0, SkBlendMode::kLastMode);
            return SkColorFilter::MakeModeFilter(color, mode);
        }
        case 2: {
            sk_sp<SkColorFilter> outer = make_fuzz_colorfilter(fuzz, depth - 1);
            if (!outer) {
                return nullptr;
            }
            sk_sp<SkColorFilter> inner = make_fuzz_colorfilter(fuzz, depth - 1);
            // makeComposed should be able to handle nullptr.
            return outer->makeComposed(std::move(inner));
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
        default:
            SkASSERT(false);
            break;
    }
    return nullptr;
}

static void fuzz_gradient_stops(Fuzz* fuzz, SkScalar* pos, int colorCount) {
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

static sk_sp<SkShader> make_fuzz_shader(Fuzz* fuzz, int depth) {
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
            img = make_fuzz_image(fuzz);
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->nextRange(&tmY, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
            }
            return img->makeShader(tmX, tmY, useMatrix ? &matrix : nullptr);
        case 4:
            bitmap = make_fuzz_bitmap(fuzz);
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->nextRange(&tmY, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
            }
            return SkShader::MakeBitmapShader(bitmap, tmX, tmY, useMatrix ? &matrix : nullptr);
        case 5:
            shader1 = make_fuzz_shader(fuzz, depth - 1);  // limit recursion.
            FuzzNiceMatrix(fuzz, &matrix);
            return shader1 ? shader1->makeWithLocalMatrix(matrix) : nullptr;
        case 6:
            shader1 = make_fuzz_shader(fuzz, depth - 1);  // limit recursion.
            colorFilter = make_fuzz_colorfilter(fuzz, depth - 1);
            return shader1 ? shader1->makeWithColorFilter(std::move(colorFilter)) : nullptr;
        case 7:
            shader1 = make_fuzz_shader(fuzz, depth - 1);  // limit recursion.
            shader2 = make_fuzz_shader(fuzz, depth - 1);
            fuzz->nextRange(&blendMode, 0, SkBlendMode::kLastMode);
            return SkShader::MakeComposeShader(std::move(shader1), std::move(shader2), blendMode);
        case 8: {
            auto pic = make_fuzz_picture(fuzz, depth - 1);
            bool useTile;
            SkRect tile;
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->nextRange(&tmY, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix, &useTile);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
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
            // Deprecated SkGaussianEdgeShader
            return nullptr;
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
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix, &usePos);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
            }
            if (usePos) {
                fuzz_gradient_stops(fuzz, pos, colorCount);
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
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix, &usePos, &center, &radius);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
            }
            if (usePos) {
                fuzz_gradient_stops(fuzz, pos, colorCount);
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
            fuzz->nextRange(&tmX, 0, SkShader::TileMode::kLast_TileMode);
            fuzz->next(&useMatrix, &usePos, &startRadius, &endRadius, &start, &end);
            fuzz->nextRange(&colorCount, 2, kMaxColors);
            fuzz->nextN(colors, colorCount);
            if (useMatrix) {
                FuzzNiceMatrix(fuzz, &matrix);
            }
            if (usePos) {
                fuzz_gradient_stops(fuzz, pos, colorCount);
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
                FuzzNiceMatrix(fuzz, &matrix);
            }
            if (usePos) {
                fuzz_gradient_stops(fuzz, pos, colorCount);
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
            SkASSERT(false);
            break;
    }
    return nullptr;
}

static sk_sp<SkPathEffect> make_fuzz_patheffect(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    uint8_t pathEffectType;
    fuzz->nextRange(&pathEffectType, 0, 8);
    switch (pathEffectType) {
        case 0: {
            return nullptr;
        }
        case 1: {
            sk_sp<SkPathEffect> first = make_fuzz_patheffect(fuzz, depth - 1);
            sk_sp<SkPathEffect> second = make_fuzz_patheffect(fuzz, depth - 1);
            return SkPathEffect::MakeSum(std::move(first), std::move(second));
        }
        case 2: {
            sk_sp<SkPathEffect> first = make_fuzz_patheffect(fuzz, depth - 1);
            sk_sp<SkPathEffect> second = make_fuzz_patheffect(fuzz, depth - 1);
            return SkPathEffect::MakeCompose(std::move(first), std::move(second));
        }
        case 3: {
            SkPath path;
            FuzzNicePath(fuzz, &path, 20);
            SkScalar advance, phase;
            fuzz->next(&advance, &phase);
            SkPath1DPathEffect::Style style;
            fuzz->nextRange(&style, 0, SkPath1DPathEffect::kLastEnum_Style);
            return SkPath1DPathEffect::Make(path, advance, phase, style);
        }
        case 4: {
            SkScalar width;
            SkMatrix matrix;
            fuzz->next(&width);
            FuzzNiceMatrix(fuzz, &matrix);
            return SkLine2DPathEffect::Make(width, matrix);
        }
        case 5: {
            SkPath path;
            FuzzNicePath(fuzz, &path, 20);
            SkMatrix matrix;
            FuzzNiceMatrix(fuzz, &matrix);
            return SkPath2DPathEffect::Make(matrix, path);
        }
        case 6: {
            SkScalar radius;
            fuzz->next(&radius);
            return SkCornerPathEffect::Make(radius);
        }
        case 7: {
            SkScalar phase;
            fuzz->next(&phase);
            SkScalar intervals[20];
            int count;
            fuzz->nextRange(&count, 0, (int)SK_ARRAY_COUNT(intervals));
            fuzz->nextN(intervals, count);
            return SkDashPathEffect::Make(intervals, count, phase);
        }
        case 8: {
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

static sk_sp<SkMaskFilter> make_fuzz_maskfilter(Fuzz* fuzz) {
    int maskfilterType;
    fuzz->nextRange(&maskfilterType, 0, 1);
    switch (maskfilterType) {
        case 0:
            return nullptr;
        case 1: {
            SkBlurStyle blurStyle;
            fuzz->nextRange(&blurStyle, 0, kLastEnum_SkBlurStyle);
            SkScalar sigma;
            fuzz->next(&sigma);
            bool respectCTM;
            fuzz->next(&respectCTM);
            return SkMaskFilter::MakeBlur(blurStyle, sigma, respectCTM);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static sk_sp<SkTypeface> make_fuzz_typeface(Fuzz* fuzz) {
    if (make_fuzz_t<bool>(fuzz)) {
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

static sk_sp<SkImageFilter> make_fuzz_imageFilter(Fuzz* fuzz, int depth);

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
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeDistantLitDiffuse(p, lightColor, surfaceScale, k,
                                                                std::move(input),
                                                                useCropRect ? &cropRect : nullptr);
        case 2:
            fuzz->next(&p, &lightColor, &surfaceScale, &k);
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakePointLitDiffuse(p, lightColor, surfaceScale, k,
                                                              std::move(input),
                                                              useCropRect ? &cropRect : nullptr);
        case 3:
            fuzz->next(&p, &q, &specularExponent, &cutoffAngle, &lightColor, &surfaceScale, &k);
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeSpotLitDiffuse(
                    p, q, specularExponent, cutoffAngle, lightColor, surfaceScale, k,
                    std::move(input), useCropRect ? &cropRect : nullptr);
        case 4:
            fuzz->next(&p, &lightColor, &surfaceScale, &k, &shininess);
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeDistantLitSpecular(p, lightColor, surfaceScale, k,
                                                                 shininess, std::move(input),
                                                                 useCropRect ? &cropRect : nullptr);
        case 5:
            fuzz->next(&p, &lightColor, &surfaceScale, &k, &shininess);
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakePointLitSpecular(p, lightColor, surfaceScale, k,
                                                               shininess, std::move(input),
                                                               useCropRect ? &cropRect : nullptr);
        case 6:
            fuzz->next(&p, &q, &specularExponent, &cutoffAngle, &lightColor, &surfaceScale, &k,
                       &shininess);
            input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkLightingImageFilter::MakeSpotLitSpecular(
                    p, q, specularExponent, cutoffAngle, lightColor, surfaceScale, k, shininess,
                    std::move(input), useCropRect ? &cropRect : nullptr);
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static void fuzz_paint(Fuzz* fuzz, SkPaint* paint, int depth);

static sk_sp<SkImageFilter> make_fuzz_imageFilter(Fuzz* fuzz, int depth) {
    if (depth <= 0) {
        return nullptr;
    }
    uint8_t imageFilterType;
    fuzz->nextRange(&imageFilterType, 0, 23);
    switch (imageFilterType) {
        case 0:
            return nullptr;
        case 1: {
            SkScalar sigmaX, sigmaY;
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            bool useCropRect;
            fuzz->next(&sigmaX, &sigmaY, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkBlurImageFilter::Make(sigmaX, sigmaY, std::move(input),
                                           useCropRect ? &cropRect : nullptr);
        }
        case 2: {
            SkMatrix matrix;
            FuzzNiceMatrix(fuzz, &matrix);
            SkFilterQuality quality;
            fuzz->nextRange(&quality, 0, SkFilterQuality::kLast_SkFilterQuality);
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkImageFilter::MakeMatrixFilter(matrix, quality, std::move(input));
        }
        case 3: {
            SkRegion region;
            SkScalar innerMin, outerMax;
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            bool useCropRect;
            fuzz->next(&region, &innerMin, &outerMax, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkAlphaThresholdFilter::Make(region, innerMin, outerMax, std::move(input),
                                                useCropRect ? &cropRect : nullptr);
        }
        case 4: {
            float k1, k2, k3, k4;
            bool enforcePMColor;
            bool useCropRect;
            fuzz->next(&k1, &k2, &k3, &k4, &enforcePMColor, &useCropRect);
            sk_sp<SkImageFilter> background = make_fuzz_imageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> foreground = make_fuzz_imageFilter(fuzz, depth - 1);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkArithmeticImageFilter::Make(k1, k2, k3, k4, enforcePMColor,
                                                 std::move(background), std::move(foreground),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 5: {
            sk_sp<SkColorFilter> cf = make_fuzz_colorfilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            bool useCropRect;
            SkImageFilter::CropRect cropRect;
            fuzz->next(&useCropRect);
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkColorFilterImageFilter::Make(std::move(cf), std::move(input),
                                                  useCropRect ? &cropRect : nullptr);
        }
        case 6: {
            sk_sp<SkImageFilter> ifo = make_fuzz_imageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> ifi = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkComposeImageFilter::Make(std::move(ifo), std::move(ifi));
        }
        case 7: {
            SkDisplacementMapEffect::ChannelSelectorType xChannelSelector, yChannelSelector;
            fuzz->nextRange(&xChannelSelector, 1, 4);
            fuzz->nextRange(&yChannelSelector, 1, 4);
            SkScalar scale;
            bool useCropRect;
            fuzz->next(&scale, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> displacement = make_fuzz_imageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> color = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                                 std::move(displacement), std::move(color),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 8: {
            SkScalar dx, dy, sigmaX, sigmaY;
            SkColor color;
            SkDropShadowImageFilter::ShadowMode shadowMode;
            fuzz->nextRange(&shadowMode, 0, 1);
            bool useCropRect;
            fuzz->next(&dx, &dy, &sigmaX, &sigmaY, &color, &useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color, shadowMode,
                                                 std::move(input),
                                                 useCropRect ? &cropRect : nullptr);
        }
        case 9:
            return SkImageSource::Make(make_fuzz_image(fuzz));
        case 10: {
            sk_sp<SkImage> image = make_fuzz_image(fuzz);
            SkRect srcRect, dstRect;
            SkFilterQuality filterQuality;
            fuzz->next(&srcRect, &dstRect);
            fuzz->nextRange(&filterQuality, 0, SkFilterQuality::kLast_SkFilterQuality);
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
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
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
            fuzz->nextRange(&tileMode, 0, SkMatrixConvolutionImageFilter::TileMode::kLast_TileMode);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkMatrixConvolutionImageFilter::Make(
                    SkISize{n, m}, kernel, gain, bias, SkIPoint{offsetX, offsetY}, tileMode,
                    convolveAlpha, std::move(input), useCropRect ? &cropRect : nullptr);
        }
        case 14: {
            sk_sp<SkImageFilter> first = make_fuzz_imageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> second = make_fuzz_imageFilter(fuzz, depth - 1);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkMergeImageFilter::Make(std::move(first), std::move(second),
                                            useCropRect ? &cropRect : nullptr);
        }
        case 15: {
            constexpr int kMaxCount = 4;
            sk_sp<SkImageFilter> ifs[kMaxCount];
            int count;
            fuzz->nextRange(&count, 1, kMaxCount);
            for (int i = 0; i < count; ++i) {
                ifs[i] = make_fuzz_imageFilter(fuzz, depth - 1);
            }
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkMergeImageFilter::Make(ifs, count, useCropRect ? &cropRect : nullptr);
        }
        case 16: {
            int rx, ry;
            fuzz->next(&rx, &ry);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
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
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
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
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkOffsetImageFilter::Make(dx, dy, std::move(input),
                                             useCropRect ? &cropRect : nullptr);
        }
        case 19: {
            SkPaint paint;
            fuzz_paint(fuzz, &paint, depth - 1);
            bool useCropRect;
            fuzz->next(&useCropRect);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            return SkPaintImageFilter::Make(paint, useCropRect ? &cropRect : nullptr);
        }
        case 20: {
            sk_sp<SkPicture> picture = make_fuzz_picture(fuzz, depth - 1);
            return SkPictureImageFilter::Make(std::move(picture));
        }
        case 21: {
            SkRect cropRect;
            fuzz->next(&cropRect);
            sk_sp<SkPicture> picture = make_fuzz_picture(fuzz, depth - 1);
            return SkPictureImageFilter::Make(std::move(picture), cropRect);
        }
        case 22: {
            SkRect src, dst;
            fuzz->next(&src, &dst);
            sk_sp<SkImageFilter> input = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkTileImageFilter::Make(src, dst, std::move(input));
        }
        case 23: {
            SkBlendMode blendMode;
            bool useCropRect;
            fuzz->next(&useCropRect);
            fuzz->nextRange(&blendMode, 0, SkBlendMode::kLastMode);
            SkImageFilter::CropRect cropRect;
            if (useCropRect) {
                fuzz->next(&cropRect);
            }
            sk_sp<SkImageFilter> bg = make_fuzz_imageFilter(fuzz, depth - 1);
            sk_sp<SkImageFilter> fg = make_fuzz_imageFilter(fuzz, depth - 1);
            return SkXfermodeImageFilter::Make(blendMode, std::move(bg), std::move(fg),
                                               useCropRect ? &cropRect : nullptr);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static sk_sp<SkImage> make_fuzz_image(Fuzz* fuzz) {
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

static SkBitmap make_fuzz_bitmap(Fuzz* fuzz) {
    SkBitmap bitmap;
    int w, h;
    fuzz->nextRange(&w, 1, 1024);
    fuzz->nextRange(&h, 1, 1024);
    if (!bitmap.tryAllocN32Pixels(w, h)) {
        SkDEBUGF("Could not allocate pixels %d x %d", w, h);
        return bitmap;
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            SkColor c;
            fuzz->next(&c);
            *bitmap.getAddr32(x, y) = SkPreMultiplyColor(c);
        }
    }
    return bitmap;
}

template <typename T, typename Min, typename Max>
inline T make_fuzz_t_range(Fuzz* fuzz, Min minv, Max maxv) {
    T value;
    fuzz->nextRange(&value, minv, maxv);
    return value;
}

static void fuzz_paint(Fuzz* fuzz, SkPaint* paint, int depth) {
    if (!fuzz || !paint || depth <= 0) {
        return;
    }

    paint->setAntiAlias(    make_fuzz_t<bool>(fuzz));
    paint->setDither(       make_fuzz_t<bool>(fuzz));
    paint->setColor(        make_fuzz_t<SkColor>(fuzz));
    paint->setBlendMode(    make_fuzz_t_range<SkBlendMode>(fuzz, 0, SkBlendMode::kLastMode));
    paint->setFilterQuality(make_fuzz_t_range<SkFilterQuality>(fuzz, 0, kLast_SkFilterQuality));
    paint->setStyle(        make_fuzz_t_range<SkPaint::Style>(fuzz, 0, 2));
    paint->setShader(       make_fuzz_shader(fuzz, depth - 1));
    paint->setPathEffect(   make_fuzz_patheffect(fuzz, depth - 1));
    paint->setMaskFilter(   make_fuzz_maskfilter(fuzz));
    paint->setImageFilter(  make_fuzz_imageFilter(fuzz, depth - 1));
    paint->setColorFilter(  make_fuzz_colorfilter(fuzz, depth - 1));

    if (paint->getStyle() != SkPaint::kFill_Style) {
        paint->setStrokeWidth(make_fuzz_t<SkScalar>(fuzz));
        paint->setStrokeMiter(make_fuzz_t<SkScalar>(fuzz));
        paint->setStrokeCap(  make_fuzz_t_range<SkPaint::Cap>(fuzz, 0, SkPaint::kLast_Cap));
        paint->setStrokeJoin( make_fuzz_t_range<SkPaint::Join>(fuzz, 0, SkPaint::kLast_Join));
    }
}

static SkFont fuzz_font(Fuzz* fuzz) {
    SkFont font;
    font.setTypeface(           make_fuzz_typeface(fuzz));
    font.setSize(               make_fuzz_t<SkScalar>(fuzz));
    font.setScaleX(             make_fuzz_t<SkScalar>(fuzz));
    font.setSkewX(              make_fuzz_t<SkScalar>(fuzz));
    font.setLinearMetrics(      make_fuzz_t<bool>(fuzz));
    font.setSubpixel(           make_fuzz_t<bool>(fuzz));
    font.setEmbeddedBitmaps(    make_fuzz_t<bool>(fuzz));
    font.setForceAutoHinting(   make_fuzz_t<bool>(fuzz));
    font.setEmbolden(           make_fuzz_t<bool>(fuzz));
    font.setHinting(            make_fuzz_t_range<SkFontHinting>(fuzz, 0, kFull_SkFontHinting));
    font.setEdging(             make_fuzz_t_range<SkFont::Edging>(fuzz, 0,
                                                      (int)SkFont::Edging::kSubpixelAntiAlias));
    return font;
}

static SkTextEncoding fuzz_paint_text_encoding(Fuzz* fuzz) {
    return make_fuzz_t_range<SkTextEncoding>(fuzz, 0, 3);
}

constexpr int kMaxGlyphCount = 30;

static SkTDArray<uint8_t> make_fuzz_text(Fuzz* fuzz, const SkFont& font, SkTextEncoding encoding) {
    SkTDArray<uint8_t> array;
    if (kGlyphID_SkTextEncoding == encoding) {
        int glyphRange = font.getTypefaceOrDefault()->countGlyphs();
        if (glyphRange == 0) {
            // Some fuzzing environments have no fonts, so empty array is the best
            // we can do.
            return array;
        }
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
    switch (encoding) {
        case kUTF8_SkTextEncoding: {
            size_t utf8len = 0;
            for (int j = 0; j < length; ++j) {
                utf8len += SkUTF::ToUTF8(buffer[j], nullptr);
            }
            char* ptr = (char*)array.append(utf8len);
            for (int j = 0; j < length; ++j) {
                ptr += SkUTF::ToUTF8(buffer[j], ptr);
            }
        } break;
        case kUTF16_SkTextEncoding: {
            size_t utf16len = 0;
            for (int j = 0; j < length; ++j) {
                utf16len += SkUTF::ToUTF16(buffer[j]);
            }
            uint16_t* ptr = (uint16_t*)array.append(utf16len * sizeof(uint16_t));
            for (int j = 0; j < length; ++j) {
                ptr += SkUTF::ToUTF16(buffer[j], ptr);
            }
        } break;
        case kUTF32_SkTextEncoding:
            memcpy(array.append(length * sizeof(SkUnichar)), buffer, length * sizeof(SkUnichar));
            break;
        default:
            SkASSERT(false);
            break;
    }
    return array;
}

static sk_sp<SkTextBlob> make_fuzz_textblob(Fuzz* fuzz) {
    SkTextBlobBuilder textBlobBuilder;
    int8_t runCount;
    fuzz->nextRange(&runCount, (int8_t)1, (int8_t)8);
    while (runCount-- > 0) {
        SkFont font;
        SkTextEncoding encoding = fuzz_paint_text_encoding(fuzz);
        font.setEdging(make_fuzz_t<bool>(fuzz) ? SkFont::Edging::kAlias : SkFont::Edging::kAntiAlias);
        SkTDArray<uint8_t> text = make_fuzz_text(fuzz, font, encoding);
        int glyphCount = font.countText(text.begin(), SkToSizeT(text.count()), encoding);
        SkASSERT(glyphCount <= kMaxGlyphCount);
        SkScalar x, y;
        const SkTextBlobBuilder::RunBuffer* buffer;
        uint8_t runType;
        fuzz->nextRange(&runType, (uint8_t)0, (uint8_t)2);
        const void* textPtr = text.begin();
        size_t textLen =  SkToSizeT(text.count());
        switch (runType) {
            case 0:
                fuzz->next(&x, &y);
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRun(font, glyphCount, x, y);
                (void)font.textToGlyphs(textPtr, textLen, encoding, buffer->glyphs, glyphCount);
                break;
            case 1:
                fuzz->next(&y);
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRunPosH(font, glyphCount, y);
                (void)font.textToGlyphs(textPtr, textLen, encoding, buffer->glyphs, glyphCount);
                fuzz->nextN(buffer->pos, glyphCount);
                break;
            case 2:
                // TODO: Test other variations of this.
                buffer = &textBlobBuilder.allocRunPos(font, glyphCount);
                (void)font.textToGlyphs(textPtr, textLen, encoding, buffer->glyphs, glyphCount);
                fuzz->nextN(buffer->pos, glyphCount * 2);
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
    return textBlobBuilder.make();
}

extern std::atomic<bool> gSkUseDeltaAA;
extern std::atomic<bool> gSkForceDeltaAA;

static void fuzz_canvas(Fuzz* fuzz, SkCanvas* canvas, int depth = 9) {
    if (!fuzz || !canvas || depth <= 0) {
        return;
    }
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    bool useDAA;
    fuzz->next(&useDAA);
    if (useDAA) {
        gSkForceDeltaAA = true;
        gSkUseDeltaAA = true;
    }
    unsigned N;
    fuzz->nextRange(&N, 0, 2000);
    for (unsigned i = 0; i < N; ++i) {
        if (fuzz->exhausted()) {
            return;
        }
        SkPaint paint;
        SkFont font;
        unsigned drawCommand;
        fuzz->nextRange(&drawCommand, 0, 53);
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
                fuzz_paint(fuzz, &paint, depth - 1);
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
                fuzz_paint(fuzz, &paint, depth - 1);
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
                if (make_fuzz_t<bool>(fuzz)) {
                    fuzz->next(&bounds);
                    saveLayerRec.fBounds = &bounds;
                }
                if (make_fuzz_t<bool>(fuzz)) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                    saveLayerRec.fPaint = &paint;
                }
                sk_sp<SkImageFilter> imageFilter;
                if (make_fuzz_t<bool>(fuzz)) {
                    imageFilter = make_fuzz_imageFilter(fuzz, depth - 1);
                    saveLayerRec.fBackdrop = imageFilter.get();
                }
                // _DumpCanvas can't handle this.
                // if (make_fuzz_t<bool>(fuzz)) {
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
                FuzzNiceMatrix(fuzz, &mat);
                canvas->concat(mat);
                break;
            }
            case 17: {
                SkMatrix mat;
                FuzzNiceMatrix(fuzz, &mat);
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
                FuzzNiceRRect(fuzz, &rr);
                fuzz->next(&doAntiAlias);
                fuzz->nextRange(&op, 0, 1);
                canvas->clipRRect(rr, (SkClipOp)op, doAntiAlias);
                break;
            }
            case 21: {
                SkPath path;
                FuzzNicePath(fuzz, &path, 30);
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
                fuzz_paint(fuzz, &paint, depth - 1);
                canvas->drawPaint(paint);
                break;
            case 24: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkCanvas::PointMode pointMode;
                fuzz->nextRange(&pointMode,
                                SkCanvas::kPoints_PointMode, SkCanvas::kPolygon_PointMode);
                size_t count;
                constexpr int kMaxCount = 30;
                fuzz->nextRange(&count, 0, kMaxCount);
                SkPoint pts[kMaxCount];
                fuzz->nextN(pts, count);
                canvas->drawPoints(pointMode, count, pts, paint);
                break;
            }
            case 25: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRect r;
                fuzz->next(&r);
                if (!r.isFinite()) {
                    break;
                }
                canvas->drawRect(r, paint);
                break;
            }
            case 26: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRegion region;
                fuzz->next(&region);
                canvas->drawRegion(region, paint);
                break;
            }
            case 27: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRect r;
                fuzz->next(&r);
                if (!r.isFinite()) {
                    break;
                }
                canvas->drawOval(r, paint);
                break;
            }
            case 28: break; // must have deleted this some time earlier
            case 29: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRRect rr;
                FuzzNiceRRect(fuzz, &rr);
                canvas->drawRRect(rr, paint);
                break;
            }
            case 30: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRRect orr, irr;
                FuzzNiceRRect(fuzz, &orr);
                FuzzNiceRRect(fuzz, &irr);
                if (orr.getBounds().contains(irr.getBounds())) {
                    canvas->drawDRRect(orr, irr, paint);
                }
                break;
            }
            case 31: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkRect r;
                SkScalar start, sweep;
                bool useCenter;
                fuzz->next(&r, &start, &sweep, &useCenter);
                canvas->drawArc(r, start, sweep, useCenter, paint);
                break;
            }
            case 32: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkPath path;
                FuzzNicePath(fuzz, &path, 60);
                canvas->drawPath(path, paint);
                break;
            }
            case 33: {
                sk_sp<SkImage> img = make_fuzz_image(fuzz);
                SkScalar left, top;
                bool usePaint;
                fuzz->next(&left, &top, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                canvas->drawImage(img.get(), left, top, usePaint ? &paint : nullptr);
                break;
            }
            case 34: {
                auto img = make_fuzz_image(fuzz);
                SkRect src, dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                canvas->drawImageRect(img, src, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 35: {
                auto img = make_fuzz_image(fuzz);
                SkIRect src;
                SkRect dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_fuzz_t<bool>(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                                : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawImageRect(img, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 36: {
                bool usePaint;
                auto img = make_fuzz_image(fuzz);
                SkRect dst;
                fuzz->next(&dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                canvas->drawImageRect(img, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 37: {
                auto img = make_fuzz_image(fuzz);
                SkIRect center;
                SkRect dst;
                bool usePaint;
                fuzz->next(&usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                if (make_fuzz_t<bool>(fuzz)) {
                    fuzz->next(&center);
                } else {  // Make valid center, see SkLatticeIter::Valid().
                    fuzz->nextRange(&center.fLeft, 0, img->width() - 1);
                    fuzz->nextRange(&center.fTop, 0, img->height() - 1);
                    fuzz->nextRange(&center.fRight, center.fLeft + 1, img->width());
                    fuzz->nextRange(&center.fBottom, center.fTop + 1, img->height());
                }
                fuzz->next(&dst);
                canvas->drawImageNine(img, center, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 38: {
                SkBitmap bitmap = make_fuzz_bitmap(fuzz);
                SkScalar left, top;
                bool usePaint;
                fuzz->next(&left, &top, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                canvas->drawBitmap(bitmap, left, top, usePaint ? &paint : nullptr);
                break;
            }
            case 39: {
                SkBitmap bitmap = make_fuzz_bitmap(fuzz);
                SkRect src, dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_fuzz_t<bool>(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                                : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(bitmap, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 40: {
                SkBitmap img = make_fuzz_bitmap(fuzz);
                SkIRect src;
                SkRect dst;
                bool usePaint;
                fuzz->next(&src, &dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_fuzz_t<bool>(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                                : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(img, src, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 41: {
                SkBitmap img = make_fuzz_bitmap(fuzz);
                SkRect dst;
                bool usePaint;
                fuzz->next(&dst, &usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                SkCanvas::SrcRectConstraint constraint =
                        make_fuzz_t<bool>(fuzz) ? SkCanvas::kStrict_SrcRectConstraint
                                                : SkCanvas::kFast_SrcRectConstraint;
                canvas->drawBitmapRect(img, dst, usePaint ? &paint : nullptr, constraint);
                break;
            }
            case 42: {
                SkBitmap img = make_fuzz_bitmap(fuzz);
                SkIRect center;
                SkRect dst;
                bool usePaint;
                fuzz->next(&usePaint);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                if (make_fuzz_t<bool>(fuzz)) {
                    fuzz->next(&center);
                } else {  // Make valid center, see SkLatticeIter::Valid().
                    if (img.width() == 0 || img.height() == 0) {
                        // bitmap may not have had its pixels initialized.
                        break;
                    }
                    fuzz->nextRange(&center.fLeft, 0, img.width() - 1);
                    fuzz->nextRange(&center.fTop, 0, img.height() - 1);
                    fuzz->nextRange(&center.fRight, center.fLeft + 1, img.width());
                    fuzz->nextRange(&center.fBottom, center.fTop + 1, img.height());
                }
                fuzz->next(&dst);
                canvas->drawBitmapNine(img, center, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 43: {
                SkBitmap img = make_fuzz_bitmap(fuzz);
                bool usePaint;
                SkRect dst;
                fuzz->next(&usePaint, &dst);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                constexpr int kMax = 6;
                int xDivs[kMax], yDivs[kMax];
                SkCanvas::Lattice lattice{xDivs, yDivs, nullptr, 0, 0, nullptr, nullptr};
                fuzz->nextRange(&lattice.fXCount, 2, kMax);
                fuzz->nextRange(&lattice.fYCount, 2, kMax);
                fuzz->nextN(xDivs, lattice.fXCount);
                fuzz->nextN(yDivs, lattice.fYCount);
                canvas->drawBitmapLattice(img, lattice, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 44: {
                auto img = make_fuzz_image(fuzz);
                bool usePaint;
                SkRect dst;
                fuzz->next(&usePaint, &dst);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                constexpr int kMax = 6;
                int xDivs[kMax], yDivs[kMax];
                SkCanvas::Lattice lattice{xDivs, yDivs, nullptr, 0, 0, nullptr, nullptr};
                fuzz->nextRange(&lattice.fXCount, 2, kMax);
                fuzz->nextRange(&lattice.fYCount, 2, kMax);
                fuzz->nextN(xDivs, lattice.fXCount);
                fuzz->nextN(yDivs, lattice.fYCount);
                canvas->drawImageLattice(img.get(), lattice, dst, usePaint ? &paint : nullptr);
                break;
            }
            case 45: {
                fuzz_paint(fuzz, &paint, depth - 1);
                font = fuzz_font(fuzz);
                SkTextEncoding encoding = fuzz_paint_text_encoding(fuzz);
                SkScalar x, y;
                fuzz->next(&x, &y);
                SkTDArray<uint8_t> text = make_fuzz_text(fuzz, font, encoding);
                canvas->drawSimpleText(text.begin(), SkToSizeT(text.count()), encoding, x, y,
                                       font, paint);
                break;
            }
            case 46: {
                // was drawPosText
                break;
            }
            case 47: {
                // was drawPosTextH
                break;
            }
            case 48: {
                // was drawtextonpath
                break;
            }
            case 49: {
                // was drawtextonpath
                break;
            }
            case 50: {
                // was drawTextRSXform
                break;
            }
            case 51: {
                sk_sp<SkTextBlob> blob = make_fuzz_textblob(fuzz);
                fuzz_paint(fuzz, &paint, depth - 1);
                SkScalar x, y;
                fuzz->next(&x, &y);
                canvas->drawTextBlob(blob, x, y, paint);
                break;
            }
            case 52: {
                SkMatrix matrix;
                bool usePaint, useMatrix;
                fuzz->next(&usePaint, &useMatrix);
                if (usePaint) {
                    fuzz_paint(fuzz, &paint, depth - 1);
                }
                if (useMatrix) {
                    FuzzNiceMatrix(fuzz, &matrix);
                }
                auto pic = make_fuzz_picture(fuzz, depth - 1);
                canvas->drawPicture(pic, useMatrix ? &matrix : nullptr,
                                    usePaint ? &paint : nullptr);
                break;
            }
            case 53: {
                fuzz_paint(fuzz, &paint, depth - 1);
                SkVertices::VertexMode vertexMode;
                SkBlendMode blendMode;
                fuzz->nextRange(&vertexMode, 0, SkVertices::kTriangleFan_VertexMode);
                fuzz->nextRange(&blendMode, 0, SkBlendMode::kLastMode);
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
                if (make_fuzz_t<bool>(fuzz)) {
                    fuzz->nextRange(&indexCount, vertexCount, vertexCount + kMaxCount);
                    for (int i = 0; i < indexCount; ++i) {
                        fuzz->nextRange(&indices[i], 0, vertexCount - 1);
                    }
                }
                canvas->drawVertices(SkVertices::MakeCopy(vertexMode, vertexCount, vertices,
                                                          useTexs ? texs : nullptr,
                                                          useColors ? colors : nullptr,
                                                          indexCount, indices),
                                     blendMode, paint);
                break;
            }
            default:
                SkASSERT(false);
                break;
        }
    }
}

static sk_sp<SkPicture> make_fuzz_picture(Fuzz* fuzz, int depth) {
    SkScalar w, h;
    fuzz->next(&w, &h);
    SkPictureRecorder pictureRecorder;
    fuzz_canvas(fuzz, pictureRecorder.beginRecording(w, h), depth - 1);
    return pictureRecorder.finishRecordingAsPicture();
}

DEF_FUZZ(NullCanvas, fuzz) {
    fuzz_canvas(fuzz, SkMakeNullCanvas().get());
}

constexpr SkISize kCanvasSize = {128, 160};

DEF_FUZZ(RasterN32Canvas, fuzz) {
    auto surface = SkSurface::MakeRasterN32Premul(kCanvasSize.width(), kCanvasSize.height());
    if (!surface || !surface->getCanvas()) { fuzz->signalBug(); }
    fuzz_canvas(fuzz, surface->getCanvas());
}

DEF_FUZZ(RasterN32CanvasViaSerialization, fuzz) {
    SkPictureRecorder recorder;
    fuzz_canvas(fuzz, recorder.beginRecording(SkIntToScalar(kCanvasSize.width()),
                                              SkIntToScalar(kCanvasSize.height())));
    sk_sp<SkPicture> pic(recorder.finishRecordingAsPicture());
    if (!pic) { fuzz->signalBug(); }
    sk_sp<SkData> data = pic->serialize();
    if (!data) { fuzz->signalBug(); }
    SkReadBuffer rb(data->data(), data->size());
    auto deserialized = SkPicturePriv::MakeFromBuffer(rb);
    if (!deserialized) { fuzz->signalBug(); }
    auto surface = SkSurface::MakeRasterN32Premul(kCanvasSize.width(), kCanvasSize.height());
    SkASSERT(surface && surface->getCanvas());
    surface->getCanvas()->drawPicture(deserialized);
}

DEF_FUZZ(ImageFilter, fuzz) {
    auto fil = make_fuzz_imageFilter(fuzz, 20);

    SkPaint paint;
    paint.setImageFilter(fil);
    SkBitmap bitmap;
    SkCanvas canvas(bitmap);
    canvas.saveLayer(SkRect::MakeWH(500, 500), &paint);
}


//SkRandom _rand;
#define SK_ADD_RANDOM_BIT_FLIPS

DEF_FUZZ(SerializedImageFilter, fuzz) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(256, 256)) {
        SkDEBUGF("Could not allocate 256x256 bitmap in SerializedImageFilter");
        return;
    }

    auto filter = make_fuzz_imageFilter(fuzz, 20);
    if (!filter) {
        return;
    }
    auto data = filter->serialize();
    const unsigned char* ptr = static_cast<const unsigned char*>(data->data());
    size_t len = data->size();
#ifdef SK_ADD_RANDOM_BIT_FLIPS
    unsigned char* p = const_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < len; ++i, ++p) {
        uint8_t j;
        fuzz->nextRange(&j, 1, 250);
        if (j == 1) { // 0.4% of the time, flip a bit or byte
            uint8_t k;
            fuzz->nextRange(&k, 1, 10);
            if (k == 1) { // Then 10% of the time, change a whole byte
                uint8_t s;
                fuzz->nextRange(&s, 0, 2);
                switch(s) {
                case 0:
                    *p ^= 0xFF; // Flip entire byte
                    break;
                case 1:
                    *p = 0xFF; // Set all bits to 1
                    break;
                case 2:
                    *p = 0x00; // Set all bits to 0
                    break;
                }
            } else {
                uint8_t s;
                fuzz->nextRange(&s, 0, 7);
                *p ^= (1 << 7);
            }
        }
    }
#endif // SK_ADD_RANDOM_BIT_FLIPS
    auto deserializedFil = SkImageFilter::Deserialize(ptr, len);

    // uncomment below to write out a serialized image filter (to make corpus
    // for -t filter_fuzz)
    // SkString s("./serialized_filters/sf");
    // s.appendU32(_rand.nextU());
    // auto file = sk_fopen(s.c_str(), SkFILE_Flags::kWrite_SkFILE_Flag);
    // sk_fwrite(data->bytes(), data->size(), file);
    // sk_fclose(file);

    SkPaint paint;
    paint.setImageFilter(deserializedFil);

    SkCanvas canvas(bitmap);
    canvas.saveLayer(SkRect::MakeWH(256, 256), &paint);
    canvas.restore();
}

#if SK_SUPPORT_GPU

static void dump_GPU_info(GrContext* context) {
    const GrGLInterface* gl = static_cast<GrGLGpu*>(context->priv().getGpu())
                                    ->glInterface();
    const GrGLubyte* output;
    GR_GL_CALL_RET(gl, output, GetString(GR_GL_RENDERER));
    SkDebugf("GL_RENDERER %s\n", (const char*) output);

    GR_GL_CALL_RET(gl, output, GetString(GR_GL_VENDOR));
    SkDebugf("GL_VENDOR %s\n", (const char*) output);

    GR_GL_CALL_RET(gl, output, GetString(GR_GL_VERSION));
    SkDebugf("GL_VERSION %s\n", (const char*) output);
}

static void fuzz_ganesh(Fuzz* fuzz, GrContext* context) {
    SkASSERT(context);
    auto surface = SkSurface::MakeRenderTarget(
            context,
            SkBudgeted::kNo,
            SkImageInfo::Make(kCanvasSize.width(), kCanvasSize.height(), kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkASSERT(surface && surface->getCanvas());
    fuzz_canvas(fuzz, surface->getCanvas());
}

DEF_FUZZ(NativeGLCanvas, fuzz) {
    sk_gpu_test::GrContextFactory f;
    GrContext* context = f.get(sk_gpu_test::GrContextFactory::kGL_ContextType);
    if (!context) {
        context = f.get(sk_gpu_test::GrContextFactory::kGLES_ContextType);
    }
    if (FLAGS_gpuInfo) {
        dump_GPU_info(context);
    }
    fuzz_ganesh(fuzz, context);
}

// This target is deprecated, NullGLContext is not well maintained.
// Please use MockGPUCanvas instead.
DEF_FUZZ(NullGLCanvas, fuzz) {
    sk_gpu_test::GrContextFactory f;
    fuzz_ganesh(fuzz, f.get(sk_gpu_test::GrContextFactory::kNullGL_ContextType));
}

DEF_FUZZ(MockGPUCanvas, fuzz) {
    sk_gpu_test::GrContextFactory f;
    fuzz_ganesh(fuzz, f.get(sk_gpu_test::GrContextFactory::kMock_ContextType));
}
#endif

DEF_FUZZ(PDFCanvas, fuzz) {
    SkNullWStream stream;
    auto doc = SkPDF::MakeDocument(&stream);
    fuzz_canvas(fuzz, doc->beginPage(SkIntToScalar(kCanvasSize.width()),
                                     SkIntToScalar(kCanvasSize.height())));
}

// not a "real" thing to fuzz, used to debug errors found while fuzzing.
DEF_FUZZ(_DumpCanvas, fuzz) {
    SkDebugCanvas debugCanvas(kCanvasSize.width(), kCanvasSize.height());
    fuzz_canvas(fuzz, &debugCanvas);
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject(); // root
    debugCanvas.toJSON(writer, dataManager, debugCanvas.getSize(), nullCanvas.get());
    writer.endObject(); // root
    writer.flush();
    sk_sp<SkData> json = stream.detachAsData();
    fwrite(json->data(), json->size(), 1, stdout);
}
