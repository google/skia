/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 @header {
#include "SkTwoPointConicalGradient.h"
 }

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

// focalParams is interpreted differently depending on if type is focal or degenerate
// when degenerate, focalParams = (r0, r0^2), so strips will use .y and kRadial will use .x
// when focal, focalParams = (1/r1, focalX = r0/(r0-r1))
// The correct parameters are calculated once in Make for each FP
in half2 focalParams;
half2 prevFocalParams;
uniform half2 uFocalParams;

@coordTransform {
    gradientMatrix
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(
        const SkTwoPointConicalGradient& shader, const GrFPArgs& args);
}

@setData(pdman) {
    if (prevFocalParams != focalParams) {
        pdman.set2f(uFocalParams, focalParams.fX, focalParams.fY);
        focalParams = prevFocalParams;
    }
}

@cppEnd {
    // .fp files do not let you reference outside enum definitions, so
    // we have to explicitly map between the two compatible enum defs
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
            const SkTwoPointConicalGradient& shader, const GrFPArgs& args) {
        GrTwoPointConicalGradientLayout::Type grType = convert_type(shader.getType());

        // The focalData struct is only valid if isFocal is true
        const SkTwoPointConicalGradient::FocalData& focalData = shader.getFocalData();
        bool isFocal = grType == Type::kFocal;

        // Calculate optimization switches from gradient specification
        bool isFocalOnCircle = isFocal && focalData.isFocalOnCircle();
        bool isWellBehaved = isFocal && focalData.isWellBehaved();
        bool isSwapped = isFocal && focalData.isSwapped();
        bool isNativelyFocal = isFocal && focalData.isNativelyFocal();

        // Type-specific calculations: isRadiusIncreasing, focalParams, and
        // the gradient matrix. However, all types start with the total inverse
        // local matrix calculated from the shader and args
        bool isRadiusIncreasing;
        SkPoint focalParams; // really just a 2D tuple
        SkMatrix matrix;

        // Initialize the base matrix
        if (!shader.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        if (isFocal) {
            isRadiusIncreasing = (1 - focalData.fFocalX) > 0;

            focalParams.set(1.0 / focalData.fR1, focalData.fFocalX);

            matrix.postConcat(shader.getGradientMatrix());
        } else if (grType == Type::kRadial) {
            SkScalar dr = shader.getDiffRadius();
            isRadiusIncreasing = dr >= 0;

            SkScalar r0 = shader.getStartRadius() / dr;
            focalParams.set(r0, r0 * r0);


            // GPU radial matrix is different from the original matrix, since
            // we map the diff radius to have |dr| = 1, so manually compute
            // the final gradient matrix here.

            // Map center to (0, 0)
            matrix.postTranslate(-shader.getStartCenter().fX, -shader.getStartCenter().fY);

            // scale |diffRadius| to 1
            matrix.postScale(1 / dr, 1 / dr);
        } else { // kStrip
            isRadiusIncreasing = false; // kStrip doesn't use this flag

            SkScalar r0 = shader.getStartRadius() / shader.getCenterX1();
            focalParams.set(r0, r0 * r0);


            matrix.postConcat(shader.getGradientMatrix());
        }


        return std::unique_ptr<GrFragmentProcessor>(new GrTwoPointConicalGradientLayout(
                matrix, grType, isRadiusIncreasing, isFocalOnCircle, isWellBehaved,
                isSwapped, isNativelyFocal, focalParams));
    }
}

void main() {
    half2 p = sk_TransformedCoords2D[0];
    half t = -1;

    @switch(type) {
        case Type::kStrip: {
            half r0_2 = uFocalParams.y;
            t = r0_2 - p.y * p.y;
            if (t >= 0) {
                t = p.x + sqrt(t);
            }
            break;
        }
        case Type::kRadial: {
            half r0 = uFocalParams.x;
            @if(isRadiusIncreasing) {
                t = length(p) - r0;
            } else {
                t = -length(p) - r0;
            }
            break;
        }
        case Type::kFocal: {
            half invR1 = uFocalParams.x;
            half fx = uFocalParams.y;

            half x_t = -1;
            @if (isFocalOnCircle) {
                x_t = dot(p, p) / p.x;
            } else if (isWellBehaved) {
                x_t = length(p) - p.x * invR1;
            } else {
                half temp = p.x * p.x - p.y * p.y;

                // Only do sqrt if temp >= 0; this is significantly slower than checking temp >= 0 in
                // the if statement that checks r(t) >= 0. But GPU may break if we sqrt a negative
                // float. (Although I havevn't observed that on any devices so far, and the old approach
                // also does sqrt negative value without a check.) If the performance is really
                // critical, maybe we should just compute the area where temp and x_t are always
                // valid and drop all these ifs.
                if (temp >= 0) {
                    @if(isSwapped || !isRadiusIncreasing) {
                        x_t = -sqrt(temp) - p.x * invR1;
                    } else {
                        x_t = sqrt(temp) - p.x * invR1;
                    }
                }
            }

            // The final calculation of t from x_t has lots of static optimizations
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

            // x_t is always valid, except for certain conditions when the
            // gradient is not well behaved, so the happy route statically
            // eliminates a branch
            @if (!isWellBehaved) {
                if (x_t <= 0.0) {
                    // x_t was invalid, so invalidate the computed t
                    t = -1;
                }
            }
            break;
        }
    }

    sk_OutColor = half4(t);
}
