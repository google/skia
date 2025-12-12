/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradient_DEFINED
#define SkGradient_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPoint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAPI.h"

class SkMatrix;

class SkGradient {
public:
    struct Interpolation {
        enum class InPremul : bool { kNo = false, kYes = true };

        enum class ColorSpace : uint8_t {
            // Default Skia behavior: interpolate in the color space of the destination surface
            kDestination,

            // https://www.w3.org/TR/css-color-4/#interpolation-space
            kSRGBLinear,
            kLab,
            kOKLab,
            // This is the same as kOKLab, except it has a simplified version of the CSS gamut
            // mapping algorithm (https://www.w3.org/TR/css-color-4/#css-gamut-mapping)
            // into Rec2020 space applied to it.
            // Warning: This space is experimental and should not be used in production.
            kOKLabGamutMap,
            kLCH,
            kOKLCH,
            // This is the same as kOKLCH, except it has the same gamut mapping applied to it
            // as kOKLabGamutMap does.
            // Warning: This space is experimental and should not be used in production.
            kOKLCHGamutMap,
            kSRGB,
            kHSL,
            kHWB,

            kDisplayP3,
            kRec2020,
            kProphotoRGB,
            kA98RGB,

            kLastColorSpace = kA98RGB,
        };
        static constexpr int kColorSpaceCount = static_cast<int>(ColorSpace::kLastColorSpace) + 1;

        enum class HueMethod : uint8_t {
            // https://www.w3.org/TR/css-color-4/#hue-interpolation
            kShorter,
            kLonger,
            kIncreasing,
            kDecreasing,

            kLastHueMethod = kDecreasing,
        };
        static constexpr int kHueMethodCount = static_cast<int>(HueMethod::kLastHueMethod) + 1;

        InPremul fInPremul = InPremul::kNo;
        ColorSpace fColorSpace = ColorSpace::kDestination;
        HueMethod fHueMethod = HueMethod::kShorter;  // Only relevant for LCH, OKLCH, HSL, or HWB

        // legacy compatibility -- remove when we can
        static Interpolation FromFlags(uint32_t flags) {
            return {flags & 1 ? InPremul::kYes : InPremul::kNo,
                    ColorSpace::kDestination,
                    HueMethod::kShorter};
        }
    };

    /**
     *  Specification for the colors in a gradient.
     *
     *  @param  colors  The span of colors for the gradient.
     *  @param  pos     Relative positions of each color across the gradient. If empty,
     *                  the the colors are distributed evenly. If this is not null, the values
     *                  must lie between 0.0 and 1.0, and be strictly increasing. If the first
     *                  value is not 0.0, then an additional color stop is added at position 0.0,
     *                  with the same color as colors[0]. If the the last value is less than 1.0,
     *                  then an additional color stop is added at position 1.0, with the same color
     *                  as colors[count - 1].
     *  @param mode     Tiling mode for the gradient.
     *  @param cs       Optional colorspace associated with the span of colors. If this is null,
     *                  the colors are treated as sRGB.
     */
    class Colors {
    public:
        Colors() {}
        Colors(SkSpan<const SkColor4f> colors,
               SkSpan<const float> pos,
               SkTileMode mode,
               sk_sp<SkColorSpace> cs = nullptr)
                : fColors(colors), fPos(pos), fColorSpace(std::move(cs)), fTileMode(mode) {
            SkASSERT(fPos.size() == 0 || fPos.size() == fColors.size());

            // throw away inconsistent inputs
            if (fPos.size() != fColors.size()) {
                fPos = {};
            }
        }

        Colors(SkSpan<const SkColor4f> colors, SkTileMode tm, sk_sp<SkColorSpace> cs = nullptr)
                : Colors(colors, {}, tm, std::move(cs)) {}

        SkSpan<const SkColor4f> colors() const { return fColors; }
        SkSpan<const float> positions() const { return fPos; }
        const sk_sp<SkColorSpace>& colorSpace() const { return fColorSpace; }
        SkTileMode tileMode() const { return fTileMode; }

    private:
        SkSpan<const SkColor4f> fColors;
        SkSpan<const float> fPos;
        sk_sp<SkColorSpace> fColorSpace;
        SkTileMode fTileMode = SkTileMode::kClamp;
    };

    SkGradient() {}
    SkGradient(const Colors& colors, const Interpolation& interp)
            : fColors(colors), fInterpolation(interp) {}

    const Colors& colors() const { return fColors; }
    const Interpolation& interpolation() const { return fInterpolation; }

private:
    Colors fColors;
    Interpolation fInterpolation;
};

namespace SkShaders {
/** Returns a shader that generates a linear gradient between the two specified points.
 *  If the inputs are invalid, this will return nullptr.
 *  @param points  Array of 2 points, the end-points of the line segment
 *  @param grad    Description of the colors and interpolation method
 *  @param lm      Optional local matrix, may be null
 */
SK_API sk_sp<SkShader> LinearGradient(const SkPoint pts[2],
                                      const SkGradient&,
                                      const SkMatrix* lm = nullptr);

/** Returns a shader that generates a radial gradient given the center and radius.
 *  @param center  The center of the circle for this gradient
 *  @param radius  Must be positive. The radius of the circle for this gradient
 *  @param grad    Description of the colors and interpolation method
 *  @param lm      Optional local matrix, may be null
 */
SK_API sk_sp<SkShader> RadialGradient(SkPoint center,
                                      float radius,
                                      const SkGradient& grad,
                                      const SkMatrix* lm = nullptr);

/** Returns a shader that generates a conical gradient given two circles, or
 *  returns null if the inputs are invalid. The gradient interprets the
 *  two circles according to the following HTML spec.
 *  http://dev.w3.org/html5/2dcontext/#dom-context-2d-createradialgradient
 *  @param start        The center of the circle for this gradient
 *  @param startRadius  Must be positive. The radius of the circle for this gradient
 *  @param end          The center of the circle for this gradient
 *  @param startRadius  Must be positive. The radius of the circle for this gradient
 *  @param grad         Description of the colors and interpolation method
 *  @param lm           Optional local matrix, may be null
 */
SK_API sk_sp<SkShader> TwoPointConicalGradient(SkPoint start,
                                               float startRadius,
                                               SkPoint end,
                                               float endRadius,
                                               const SkGradient& grad,
                                               const SkMatrix* lm = nullptr);

/** Returns a shader that generates a sweep gradient given a center.
 *  The shader accepts negative angles and angles larger than 360, draws
 *  between 0 and 360 degrees, similar to the CSS conic-gradient
 *  semantics. 0 degrees means horizontal positive x axis. The start angle
 *  must be less than the end angle, otherwise a null pointer is
 *  returned. If color stops do not contain 0 and 1 but are within this
 *  range, the respective outer color stop is repeated for 0 and 1. Color
 *  stops less than 0 are clamped to 0, and greater than 1 are clamped to 1.
 *  @param center      The center of the sweep
 *  @param startAngle  Start of the angular range, corresponding to pos == 0.
 *  @param endAngle    End of the angular range, corresponding to pos == 1.
 *  @param grad        Description of the colors and interpolation method
 *  @param lm          Optional local matrix, may be null
 */
SK_API sk_sp<SkShader> SweepGradient(SkPoint center,
                                     float startAngle,
                                     float endAngle,
                                     const SkGradient&,
                                     const SkMatrix* lm = nullptr);
static inline sk_sp<SkShader> SweepGradient(SkPoint center,
                                     const SkGradient& grad,
                                     const SkMatrix* lm = nullptr) {
    return SweepGradient(center, 0, 360, grad, lm);
}

}  // namespace SkShaders

#endif
