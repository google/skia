/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkHighContrastFilter_DEFINED
#define SkHighContrastFilter_DEFINED

#include "SkColorFilter.h"
#include "SkPaint.h"

enum class HighContrastInvertStyle {
    kNoInvert,
    kInvertBrightness,
    kInvertLightness,
};

/**
 *  Configuration struct for SkHighContrastFilter.
 *
 *  Provides transformations to improve contrast for users with low vision.
 */
struct SkHighContrastConfig {
    SkHighContrastConfig() {
        fGrayscale = false;
        fInvertStyle = HighContrastInvertStyle::kInvertLightness;
        fExponent = 0.5f;
        fContrast = 0.0f;
    }

    SkHighContrastConfig(bool grayscale,
                         HighContrastInvertStyle invertStyle,
                         SkScalar exponent,
                         SkScalar contrast)
        : fGrayscale(grayscale),
          fInvertStyle(invertStyle),
          fExponent(exponent),
          fContrast(contrast) {}

    // If true, the color will be converted to grayscale.
    bool fGrayscale;

    // Whether to invert brightness, lightness, or neither.
    HighContrastInvertStyle fInvertStyle;

    // After optionally converting to grayscale and inverting, the color will be
    // raised to this exponent.  A value of 0.5 helps undo the implicit gamma
    // of the source.
    SkScalar  fExponent;

    // After grayscale, inverting, and raising to an exponent, the contrast
    // can be adjusted linearly.
    SkScalar  fContrast;
};

/**
 *  Color filter that provides transformations to improve contrast
 *  for users with low vision.
 *
 *  Applies the following transformations in this order. Each of these
 *  can be configured using SkHighContrastConfig.
 *
 *    - Conversion to grayscale
 *    - Color inversion (either in RGB or HSL space)
 *    - Raising to an exponent (gamma correction needed after inversion)
 *    - Increasing the resulting contrast.
 *
 */
class SK_API SkHighContrastFilter : public SkColorFilter {
public:
    static void Apply(const SkHighContrastConfig& config,
                      unsigned char* r,
                      unsigned char* g,
                      unsigned char* b);

    static SkColor Apply(const SkHighContrastConfig& config, SkColor srcColor);

    static SkPaint Apply(const SkHighContrastConfig& config, const SkPaint& src);

    static sk_sp<SkColorFilter> Make(const SkHighContrastConfig& config);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
