/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkHighContrastFilter_DEFINED
#define SkHighContrastFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"

/**
 *  Configuration struct for SkHighContrastFilter.
 *
 *  Provides transformations to improve contrast for users with low vision.
 */
struct SkHighContrastConfig {
    enum class InvertStyle {
        kNoInvert,
        kInvertBrightness,
        // Inverts the lightness of the color using HSL colorspace.
        kInvertLightness,
        // Useful only if fUseOptimized is true.
        kInvertLightness_YCbCrBT601,

        kFirst = kNoInvert,
        kLast = kInvertLightness_YCbCrBT601
    };

    SkHighContrastConfig() {
        fUseOptimized = false;
        fGrayscale = false;
        fInvertStyle = InvertStyle::kNoInvert;
        fContrast = 0.0f;
    }

    SkHighContrastConfig(bool grayscale, InvertStyle invertStyle, SkScalar contrast)
            : fUseOptimized(false)
            , fGrayscale(grayscale)
            , fInvertStyle(invertStyle)
            , fContrast(contrast) {}

    // Returns true if all of the fields are set within the valid range.
    bool isValid() const {
        return fInvertStyle >= InvertStyle::kFirst && fInvertStyle <= InvertStyle::kLast &&
               fContrast >= -1.0 && fContrast <= 1.0;
    }

    // If true, SkHighContrastFilter uses optimized version. By default,
    // value of this is false. Once stable, it would be made true.
    bool fUseOptimized;

    // If true, the color will be converted to grayscale.
    bool fGrayscale;

    // Whether to invert brightness, lightness, or neither.
    InvertStyle fInvertStyle;

    // After grayscale and inverting, the contrast can be adjusted linearly.
    // The valid range is -1.0 through 1.0, where 0.0 is no adjustment.
    SkScalar  fContrast;
};

// class Color4x4Matrix;
struct Color4x1Matrix {
    Color4x1Matrix() : fMat{1, 1, 1, 1} {}
    Color4x1Matrix(float m00, float m10, float m20, float m30) : fMat{m00, m10, m20, m30} {}
    Color4x1Matrix(const Color4x1Matrix& other) = default;
    float fMat[4];
};

struct Color4x4Matrix {
    Color4x4Matrix() : fMat{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1} {}

    Color4x4Matrix(float m00,
                   float m01,
                   float m02,
                   float m03,
                   float m10,
                   float m11,
                   float m12,
                   float m13,
                   float m20,
                   float m21,
                   float m22,
                   float m23,
                   float m30,
                   float m31,
                   float m32,
                   float m33)
            : fMat{m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33} {
    }

    Color4x4Matrix(const Color4x4Matrix& other) = default;

    float fMat[16];
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
 *    - Increasing the resulting contrast.
 *
 * Calling SkHighContrastFilter::Make will return nullptr if the config is
 * not valid, e.g. if you try to call it with a contrast outside the range of
 * -1.0 to 1.0.
 */

class SK_API SkHighContrastFilter : public SkColorFilter {
public:
    // Returns the filter, or nullptr if the config is invalid.
    static sk_sp<SkColorFilter> Make(const SkHighContrastConfig& config);

    // TODO(prashant.n): Make ctor private.
    SkHighContrastFilter(const SkHighContrastConfig& config);

    ~SkHighContrastFilter() override {}

    SkColor filterColorWithoutPipeline(SkColor) const;

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
    SK_FLATTENABLE_HOOKS(SkHighContrastFilter)

    void buildTransformationMatrix();

    SkHighContrastConfig fConfig;

    std::unique_ptr<Color4x4Matrix> fTransformationMatrix;

    typedef SkColorFilter INHERITED;
};

#endif
