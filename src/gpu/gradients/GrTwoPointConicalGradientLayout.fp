/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Equivalent to SkTwoPointConicalGradient::Type
enum class Type {
    kRadial, kStrip, kFocal
};

in half4x4 gradientMatrix;

layout(key) in Type type;
layout(key) in bool isRadiusIncreasing;

// Focal-specific optimizations
layout(key) in bool isFocalOnCircle;
layout(key) in bool isWellBehaved;
layout(key) in bool isSwapped;
layout(key) in bool isNativelyFocal;

// focalParams is interpreted differently depending on if type is focal or degenerate when
// degenerate, focalParams = (r0, r0^2), so strips will use .y and kRadial will use .x when focal,
// focalParams = (1/r1, focalX = r0/(r0-r1)) The correct parameters are calculated once in Make for
// each FP
layout(tracked) in uniform half2 focalParams;

@coordTransform {
    gradientMatrix
}

void main() {
    // p typed as a float2 is intentional; while a half2 is adequate for most normal cases in the
    // two point conic gradient's coordinate system, when the gradient is composed with a local
    // perspective matrix, certain out-of-bounds regions become ill behaved on mobile devices.
    // On desktops, they are properly clamped after the fact, but on many Adreno GPUs the
    // calculations of t and x_t below overflow and produce an incorrect interpolant (which then
    // renders the wrong border color sporadically). Increasing precition alleviates that issue.
    float2 p = sk_TransformedCoords2D[0];
    float t = -1;
    half v = 1; // validation flag, set to negative to discard fragment later

    @switch(type) {
        case Type::kStrip: {
            half r0_2 = focalParams.y;
            t = r0_2 - p.y * p.y;
            if (t >= 0) {
                t = p.x + sqrt(t);
            } else {
                v = -1;
            }
        }
        break;
        case Type::kRadial: {
            half r0 = focalParams.x;
            @if(isRadiusIncreasing) {
                t = length(p) - r0;
            } else {
                t = -length(p) - r0;
            }
        }
        break;
        case Type::kFocal: {
            half invR1 = focalParams.x;
            half fx = focalParams.y;

            float x_t = -1;
            @if (isFocalOnCircle) {
                x_t = dot(p, p) / p.x;
            } else if (isWellBehaved) {
                x_t = length(p) - p.x * invR1;
            } else {
                float temp = p.x * p.x - p.y * p.y;

                // Only do sqrt if temp >= 0; this is significantly slower than checking temp >= 0
                // in the if statement that checks r(t) >= 0. But GPU may break if we sqrt a
                // negative float. (Although I havevn't observed that on any devices so far, and the
                // old approach also does sqrt negative value without a check.) If the performance
                // is really critical, maybe we should just compute the area where temp and x_t are
                // always valid and drop all these ifs.
                if (temp >= 0) {
                    @if(isSwapped || !isRadiusIncreasing) {
                        x_t = -sqrt(temp) - p.x * invR1;
                    } else {
                        x_t = sqrt(temp) - p.x * invR1;
                    }
                }
            }

            // The final calculation of t from x_t has lots of static optimizations but only do them
            // when x_t is positive (which can be assumed true if isWellBehaved is true)
            @if (!isWellBehaved) {
                // This will still calculate t even though it will be ignored later in the pipeline
                // to avoid a branch
                if (x_t <= 0.0) {
                    v = -1;
                }
            }
            @if (isRadiusIncreasing) {
                @if (isNativelyFocal) {
                    t = x_t;
                } else {
                    t = x_t + fx;
                }
            } else {
                @if (isNativelyFocal) {
                    t = -x_t;
                } else {
                    t = -x_t + fx;
                }
            }

            @if(isSwapped) {
                t = 1 - t;
            }
        }
        break;
    }

    sk_OutColor = half4(half(t), v, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////

@header {
    #include "src/gpu/gradients/GrGradientShader.h"
    #include "src/shaders/gradients/SkTwoPointConicalGradient.h"
}

// The 2 point conical gradient can reject a pixel so it does change opacity
// even if the input was opaque, so disable that optimization
@optimizationFlags {
    kNone_OptimizationFlags
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkTwoPointConicalGradient& gradient,
                                                     const GrFPArgs& args);
}

@cppEnd {
    // .fp files do not let you reference outside enum definitions, so we have to explicitly map
    // between the two compatible enum defs
    GrTwoPointConicalGradientLayout::Type convert_type(
            SkTwoPointConicalGradient::Type type) {
        switch(type) {
            case SkTwoPointConicalGradient::Type::kRadial:
                return GrTwoPointConicalGradientLayout::Type::kRadial;
            case SkTwoPointConicalGradient::Type::kStrip:
                return GrTwoPointConicalGradientLayout::Type::kStrip;
            case SkTwoPointConicalGradient::Type::kFocal:
                return GrTwoPointConicalGradientLayout::Type::kFocal;
        }
        SkDEBUGFAIL("Should not be reachable");
        return GrTwoPointConicalGradientLayout::Type::kRadial;
    }

    std::unique_ptr<GrFragmentProcessor> GrTwoPointConicalGradientLayout::Make(
            const SkTwoPointConicalGradient& grad, const GrFPArgs& args) {
        GrTwoPointConicalGradientLayout::Type grType = convert_type(grad.getType());

        // The focalData struct is only valid if isFocal is true
        const SkTwoPointConicalGradient::FocalData& focalData = grad.getFocalData();
        bool isFocal = grType == Type::kFocal;

        // Calculate optimization switches from gradient specification
        bool isFocalOnCircle = isFocal && focalData.isFocalOnCircle();
        bool isWellBehaved = isFocal && focalData.isWellBehaved();
        bool isSwapped = isFocal && focalData.isSwapped();
        bool isNativelyFocal = isFocal && focalData.isNativelyFocal();

        // Type-specific calculations: isRadiusIncreasing, focalParams, and the gradient matrix.
        // However, all types start with the total inverse local matrix calculated from the shader
        // and args
        bool isRadiusIncreasing;
        SkPoint focalParams; // really just a 2D tuple
        SkMatrix matrix;

        // Initialize the base matrix
        if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        if (isFocal) {
            isRadiusIncreasing = (1 - focalData.fFocalX) > 0;

            focalParams.set(1.0 / focalData.fR1, focalData.fFocalX);

            matrix.postConcat(grad.getGradientMatrix());
        } else if (grType == Type::kRadial) {
            SkScalar dr = grad.getDiffRadius();
            isRadiusIncreasing = dr >= 0;

            SkScalar r0 = grad.getStartRadius() / dr;
            focalParams.set(r0, r0 * r0);


            // GPU radial matrix is different from the original matrix, since we map the diff radius
            // to have |dr| = 1, so manually compute the final gradient matrix here.

            // Map center to (0, 0)
            matrix.postTranslate(-grad.getStartCenter().fX, -grad.getStartCenter().fY);

            // scale |diffRadius| to 1
            matrix.postScale(1 / dr, 1 / dr);
        } else { // kStrip
            isRadiusIncreasing = false; // kStrip doesn't use this flag

            SkScalar r0 = grad.getStartRadius() / grad.getCenterX1();
            focalParams.set(r0, r0 * r0);


            matrix.postConcat(grad.getGradientMatrix());
        }


        return std::unique_ptr<GrFragmentProcessor>(new GrTwoPointConicalGradientLayout(
                matrix, grType, isRadiusIncreasing, isFocalOnCircle, isWellBehaved,
                isSwapped, isNativelyFocal, focalParams));
    }
}

//////////////////////////////////////////////////////////////////////////////

@test(d) {
    SkScalar scale = GrGradientShader::RandomParams::kGradientScale;
    SkScalar offset = scale / 32.0f;

    SkPoint center1 = {d->fRandom->nextRangeScalar(0.0f, scale),
                       d->fRandom->nextRangeScalar(0.0f, scale)};
    SkPoint center2 = {d->fRandom->nextRangeScalar(0.0f, scale),
                       d->fRandom->nextRangeScalar(0.0f, scale)};
    SkScalar radius1 = d->fRandom->nextRangeScalar(0.0f, scale);
    SkScalar radius2 = d->fRandom->nextRangeScalar(0.0f, scale);

    constexpr int   kTestTypeMask           = (1 << 2) - 1,
                    kTestNativelyFocalBit   = (1 << 2),
                    kTestFocalOnCircleBit   = (1 << 3),
                    kTestSwappedBit         = (1 << 4);
                    // We won't treat isWellDefined and isRadiusIncreasing specially because they
                    // should have high probability to be turned on and off as we're getting random
                    // radii and centers.

    int mask = d->fRandom->nextU();
    int type = mask & kTestTypeMask;
    if (type == static_cast<int>(Type::kRadial)) {
        center2 = center1;
        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += offset;
        }
    } else if (type == static_cast<int>(Type::kStrip)) {
        radius1 = SkTMax(radius1, .1f); // Make sure that the radius is non-zero
        radius2 = radius1;
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += offset;
        }
    } else { // kFocal_Type
        // Make sure that the centers are different
        if (SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
            center2.fX += offset;
        }

        if (kTestNativelyFocalBit & mask) {
            radius1 = 0;
        }
        if (kTestFocalOnCircleBit & mask) {
            radius2 = radius1 + SkPoint::Distance(center1, center2);
        }
        if (kTestSwappedBit & mask) {
            std::swap(radius1, radius2);
            radius2 = 0;
        }

        // Make sure that the radii are different
        if (SkScalarNearlyZero(radius1 - radius2)) {
            radius2 += offset;
        }
    }

    if (SkScalarNearlyZero(radius1 - radius2) &&
            SkScalarNearlyZero(SkPoint::Distance(center1, center2))) {
        radius2 += offset; // make sure that we're not degenerated
    }

    GrGradientShader::RandomParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeTwoPointConical(center1, radius1, center2, radius2,
                                              params.fColors4f, params.fColorSpace, params.fStops,
                                              params.fColorCount, params.fTileMode) :
        SkGradientShader::MakeTwoPointConical(center1, radius1, center2, radius2,
                                              params.fColors, params.fStops,
                                              params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());

    GrAlwaysAssert(fp);
    return fp;
}
