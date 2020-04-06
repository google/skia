/*
 * Copyright 2020 Chromium Authors.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDarkModeFilter_DEFINED
#define SkDarkModeFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"

/**
 *  Configuration struct for SkDarkModeFilter.
 *
 *  Provides transformations for enabling dark mode.
 */
struct SkDarkModeConfig {
    enum class InvertStyle {
        kNoInvert,
        kInvertBrightness,
        kInvertLightness,

        kLast = kInvertLightness
    };

    SkDarkModeConfig() {
        fGrayscale = false;
        fInvertStyle = InvertStyle::kNoInvert;
        fContrast = 0.0f;
    }

    SkDarkModeConfig(bool grayscale, InvertStyle invertStyle, SkScalar contrast)
            : fGrayscale(grayscale), fInvertStyle(invertStyle), fContrast(contrast) {}

    // Returns true if all of the fields are set within the valid range.
    bool isValid() const {
        return fInvertStyle >= InvertStyle::kNoInvert &&
               fInvertStyle <= InvertStyle::kInvertLightness && fContrast >= -1.0 &&
               fContrast <= 1.0;
    }

    // If true, the color will be converted to grayscale.
    bool fGrayscale;

    // Whether to invert brightness, lightness, or neither.
    InvertStyle fInvertStyle;

    // After grayscale and inverting, the contrast can be adjusted linearly.
    // The valid range is -1.0 through 1.0, where 0.0 is no adjustment.
    SkScalar fContrast;
};

/**
 *  Color filter that provides transformations for dark mode.
 *
 *  Applies the following transformations in this order. Each of these
 *  can be configured using SkDarkModeConfig.
 *
 *    - Conversion to grayscale
 *    - Color inversion (either in RGB or YCbCr space)
 *    - Increasing the resulting contrast.
 *
 * Calling SkDarkModeFilter::Make will return nullptr if the config is
 * not valid, e.g. if you try to call it with a contrast outside the range of
 * -1.0 to 1.0.
 */

class SkDarkModeFilter : public SkColorFilter {
public:
    // TODO(prashant.n) Make ctor private.
    SkDarkModeFilter(const SkDarkModeConfig& config);

    // Returns the filter, or nullptr if the config is invalid.
    static sk_sp<SkColorFilter> Make(const SkDarkModeConfig& config);

    ~SkDarkModeFilter() override {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             const GrColorInfo&) const override;
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;
    skvm::Color onProgram(skvm::Builder*,
                          skvm::Color,
                          SkColorSpace*,
                          skvm::Uniforms*,
                          SkArenaAlloc*) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkDarkModeFilter)

    SkDarkModeConfig fConfig;

    typedef SkColorFilter INHERITED;
};

#endif
