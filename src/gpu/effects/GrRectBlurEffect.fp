/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
#include <cmath>
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/effects/GrTextureEffect.h"
}

in fragmentProcessor inputFP;
in float4 rect;

layout(key) bool highPrecision = abs(rect.x) > 16000 || abs(rect.y) > 16000 ||
                                 abs(rect.z) > 16000 || abs(rect.w) > 16000;

layout(when= highPrecision) uniform float4 rectF;
layout(when=!highPrecision) uniform half4  rectH;

layout(key) in bool applyInvVM;
layout(when=applyInvVM) in uniform float3x3 invVM;

// Effect that is a LUT for integral of normal distribution. The value at x:[0,6*sigma] is the
// integral from -inf to (3*sigma - x). I.e. x is mapped from [0, 6*sigma] to [3*sigma to -3*sigma].
// The flip saves a reversal in the shader.
in fragmentProcessor integral;

// There is a fast variant of the effect that does 2 texture lookups and a more general one for
// wider blurs relative to rect sizes that does 4.
layout(key) in bool isFast;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
            kCompatibleWithCoverageAsAlpha_OptimizationFlag
}

@samplerParams(integral) {
    samplerParams
}

@class {
static std::unique_ptr<GrFragmentProcessor> MakeIntegralFP(GrRecordingContext* rContext,
                                                           float sixSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sixSigma / 6.f));
    auto threadSafeCache = rContext->priv().threadSafeCache();

    int width = SkGpuBlurUtils::CreateIntegralTable(sixSigma, nullptr);

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
    builder[0] = width;
    builder.finish();

    SkMatrix m = SkMatrix::Scale(width/sixSigma, 1.f);

    GrSurfaceProxyView view = threadSafeCache->find(key);

    if (view) {
        SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(
                std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
    }

    SkBitmap bitmap;
    if (!SkGpuBlurUtils::CreateIntegralTable(sixSigma, &bitmap)) {
        return {};
    }

    GrBitmapTextureMaker maker(rContext, bitmap, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    view = maker.view(GrMipmapped::kNo);
    if (!view) {
        return {};
    }

    view = threadSafeCache->add(key, view);

    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    return GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
}
}

@make {
     static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                      GrRecordingContext* context,
                                                      const GrShaderCaps& caps,
                                                      const SkRect& srcRect,
                                                      const SkMatrix& viewMatrix,
                                                      float transformedSigma) {
         SkASSERT(viewMatrix.preservesRightAngles());
         SkASSERT(srcRect.isSorted());

         if (SkGpuBlurUtils::IsEffectivelyZeroSigma(transformedSigma)) {
             // No need to blur the rect
             return inputFP;
         }

         SkMatrix invM;
         SkRect rect;
         if (viewMatrix.rectStaysRect()) {
             invM = SkMatrix::I();
             // We can do everything in device space when the src rect projects to a rect in device
             // space.
             SkAssertResult(viewMatrix.mapRect(&rect, srcRect));
         } else {
             // The view matrix may scale, perhaps anisotropically. But we want to apply our device
             // space "transformedSigma" to the delta of frag coord from the rect edges. Factor out
             // the scaling to define a space that is purely rotation/translation from device space
             // (and scale from src space) We'll meet in the middle: pre-scale the src rect to be in
             // this space and then apply the inverse of the rotation/translation portion to the
             // frag coord.
             SkMatrix m;
             SkSize scale;
             if (!viewMatrix.decomposeScale(&scale, &m)) {
                 return nullptr;
             }
             if (!m.invert(&invM)) {
                 return nullptr;
             }
             rect = {srcRect.left()   * scale.width(),
                     srcRect.top()    * scale.height(),
                     srcRect.right()  * scale.width(),
                     srcRect.bottom() * scale.height()};
         }

         if (!caps.floatIs32Bits()) {
             // We promote the math that gets us into the Gaussian space to full float when the rect
             // coords are large. If we don't have full float then fail. We could probably clip the
             // rect to an outset device bounds instead.
             if (SkScalarAbs(rect.fLeft)   > 16000.f ||
                 SkScalarAbs(rect.fTop)    > 16000.f ||
                 SkScalarAbs(rect.fRight)  > 16000.f ||
                 SkScalarAbs(rect.fBottom) > 16000.f) {
                    return nullptr;
             }
         }

         const float sixSigma = 6 * transformedSigma;
         std::unique_ptr<GrFragmentProcessor> integral = MakeIntegralFP(context, sixSigma);
         if (!integral) {
             return nullptr;
         }

         // In the fast variant we think of the midpoint of the integral texture as aligning
         // with the closest rect edge both in x and y. To simplify texture coord calculation we
         // inset the rect so that the edge of the inset rect corresponds to t = 0 in the texture.
         // It actually simplifies things a bit in the !isFast case, too.
         float threeSigma = sixSigma / 2;
         SkRect insetRect = {rect.left()   + threeSigma,
                             rect.top()    + threeSigma,
                             rect.right()  - threeSigma,
                             rect.bottom() - threeSigma};

         // In our fast variant we find the nearest horizontal and vertical edges and for each
         // do a lookup in the integral texture for each and multiply them. When the rect is
         // less than 6 sigma wide then things aren't so simple and we have to consider both the
         // left and right edge of the rectangle (and similar in y).
         bool isFast = insetRect.isSorted();
         return std::unique_ptr<GrFragmentProcessor>(new GrRectBlurEffect(std::move(inputFP),
                                                                          insetRect,
                                                                          !invM.isIdentity(),
                                                                          invM,
                                                                          std::move(integral),
                                                                          isFast));
     }
}

half4 main() {
    half xCoverage, yCoverage;
    float2 pos = sk_FragCoord.xy;
    @if (applyInvVM) {
        // It'd be great if we could lift this to the VS.
        pos = (invVM*float3(pos,1)).xy;
    }
    @if (isFast) {
        // Get the smaller of the signed distance from the frag coord to the left and right
        // edges and similar for y.
        // The integral texture goes "backwards" (from 3*sigma to -3*sigma), So, the below
        // computations align the left edge of the integral texture with the inset rect's edge
        // extending outward 6 * sigma from the inset rect.
        half2 xy;
        @if (highPrecision) {
            xy = max(half2(rectF.LT - pos), half2(pos - rectF.RB));
       } else {
            xy = max(half2(rectH.LT - pos), half2(pos - rectH.RB));
        }
        xCoverage = sample(integral, half2(xy.x, 0.5)).a;
        yCoverage = sample(integral, half2(xy.y, 0.5)).a;
    } else {
        // We just consider just the x direction here. In practice we compute x and y separately
        // and multiply them together.
        // We define our coord system so that the point at which we're evaluating a kernel
        // defined by the normal distribution (K) at 0. In this coord system let L be left
        // edge and R be the right edge of the rectangle.
        // We can calculate C by integrating K with the half infinite ranges outside the L to R
        // range and subtracting from 1:
        //   C = 1 - <integral of K from from -inf to  L> - <integral of K from R to inf>
        // K is symmetric about x=0 so:
        //   C = 1 - <integral of K from from -inf to  L> - <integral of K from -inf to -R>

        // The integral texture goes "backwards" (from 3*sigma to -3*sigma) which is factored
        // in to the below calculations.
        // Also, our rect uniform was pre-inset by 3 sigma from the actual rect being blurred,
        // also factored in.
        half4 rect;
        @if (highPrecision) {
            rect.LT = half2(rectF.LT - pos);
            rect.RB = half2(pos - rectF.RB);
        } else {
            rect.LT = half2(rectH.LT - pos);
            rect.RB = half2(pos - rectH.RB);
        }
        xCoverage = 1 - sample(integral, half2(rect.L, 0.5)).a
                      - sample(integral, half2(rect.R, 0.5)).a;
        yCoverage = 1 - sample(integral, half2(rect.T, 0.5)).a
                      - sample(integral, half2(rect.B, 0.5)).a;
    }
    return sample(inputFP) * xCoverage * yCoverage;
}

@setData(pdman) {
    float r[] {rect.fLeft, rect.fTop, rect.fRight, rect.fBottom};
    pdman.set4fv(highPrecision ? rectF : rectH, 1, r);
}

@test(data) {
    float sigma = data->fRandom->nextRangeF(3, 8);
    int x = data->fRandom->nextRangeF(1, 200);
    int y = data->fRandom->nextRangeF(1, 200);
    float width = data->fRandom->nextRangeF(200, 300);
    float height = data->fRandom->nextRangeF(200, 300);
    SkMatrix vm = GrTest::TestMatrixPreservesRightAngles(data->fRandom);
    auto rect = SkRect::MakeXYWH(x, y, width, height);
    return GrRectBlurEffect::Make(data->inputFP(), data->context(), *data->caps()->shaderCaps(),
                                  rect, vm, sigma);
}
