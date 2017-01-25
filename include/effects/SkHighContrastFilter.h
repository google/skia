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

enum HighContrastInvertStyle {
    kNoInvert,
    kInvertBrightness,
    kInvertLightness,
};

struct SkHighContrastConfig {
    SkHighContrastConfig(
          bool grayscale, uint32_t invertStyle, SkScalar exponent, SkScalar contrast)
        : fGrayscale(grayscale),
          fInvertStyle(invertStyle),
          fExponent(exponent),
          fContrast(contrast) {}
    SkHighContrastConfig() {}

    // If true, the color will be converted to grayscale.
    bool      fGrayscale = false;

    // Whether to invert brightness, lightness, or neither.
    uint32_t  fInvertStyle = kInvertLightness;

    // After optionally converting to grayscale and inverting, the color will be
    // raised to this exponent.  A value of 0.5 helps undo the implicit gamma
    // of the source.
    SkScalar  fExponent = 0.5;

    // After grayscale, inverting, and raising to an exponent, the contrast
    // can be adjusted linearly.
    SkScalar  fContrast = 0.0;
};

class SK_API SkHighContrastFilter : public SkColorFilter {
public:
    static void Apply(const SkHighContrastConfig& config,
                      unsigned char* r,
                      unsigned char* g,
                      unsigned char* b);

    static SkColor Apply(const SkHighContrastConfig& config, SkColor srcColor);

    static SkPaint Apply(const SkHighContrastConfig& config, const SkPaint& src);

    /**
     *
     */
    static sk_sp<SkColorFilter> Make(const SkHighContrastConfig& config);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
