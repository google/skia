/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/AnalyticBlurMask.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkPoint_impl.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/sksl/SkSLUtil.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace skgpu::graphite {

namespace {

std::optional<Rect> outset_bounds(const SkMatrix& localToDevice,
                                  float devSigma,
                                  const SkRect& srcRect) {
    float outsetX = 3.0f * devSigma;
    float outsetY = 3.0f * devSigma;
    if (localToDevice.isScaleTranslate()) {
        outsetX /= std::fabs(localToDevice.getScaleX());
        outsetY /= std::fabs(localToDevice.getScaleY());
    } else {
        SkSize scale;
        if (!localToDevice.decomposeScale(&scale, nullptr)) {
            return std::nullopt;
        }
        outsetX /= scale.width();
        outsetY /= scale.height();
    }
    return srcRect.makeOutset(outsetX, outsetY);
}

}  // anonymous namespace

std::optional<AnalyticBlurMask> AnalyticBlurMask::Make(Recorder* recorder,
                                                       const Transform& localToDeviceTransform,
                                                       float deviceSigma,
                                                       const SkRRect& srcRRect) {
    // TODO: Implement SkMatrix functionality used below for Transform.
    SkMatrix localToDevice = localToDeviceTransform;

    if (srcRRect.isRect() && localToDevice.preservesRightAngles()) {
        return MakeRect(recorder, localToDevice, deviceSigma, srcRRect.rect());
    }

    SkRRect devRRect;
    const bool devRRectIsValid = srcRRect.transform(localToDevice, &devRRect);
    if (devRRectIsValid && SkRRectPriv::IsCircle(devRRect)) {
        return MakeCircle(recorder, localToDevice, deviceSigma, srcRRect.rect(), devRRect.rect());
    }

    // A local-space circle transformed by a rotation matrix will fail SkRRect::transform since it
    // only supports scale + translate matrices, but is still a valid circle that can be blurred.
    if (SkRRectPriv::IsCircle(srcRRect) && localToDevice.isSimilarity()) {
        const SkRect srcRect = srcRRect.rect();
        const SkPoint devCenter = localToDevice.mapPoint(srcRect.center());
        const float devRadius = localToDevice.mapVector(0.0f, srcRect.width() / 2.0f).length();
        const SkRect devRect = {devCenter.x() - devRadius,
                                devCenter.y() - devRadius,
                                devCenter.x() + devRadius,
                                devCenter.y() + devRadius};
        return MakeCircle(recorder, localToDevice, deviceSigma, srcRect, devRect);
    }

    if (devRRectIsValid && SkRRectPriv::IsSimpleCircular(devRRect) &&
        localToDevice.isScaleTranslate()) {
        return MakeRRect(recorder, localToDevice, deviceSigma, srcRRect, devRRect);
    }

    return std::nullopt;
}

std::optional<AnalyticBlurMask> AnalyticBlurMask::MakeRect(Recorder* recorder,
                                                           const SkMatrix& localToDevice,
                                                           float devSigma,
                                                           const SkRect& srcRect) {
    SkASSERT(srcRect.isSorted());

    SkRect devRect;
    SkMatrix devToScaledShape;
    if (localToDevice.rectStaysRect()) {
        // We can do everything in device space when the src rect projects to a rect in device
        // space.
        SkAssertResult(localToDevice.mapRect(&devRect, srcRect));

    } else {
        // The view matrix may scale, perhaps anisotropically. But we want to apply our device space
        // sigma to the delta of frag coord from the rect edges. Factor out the scaling to define a
        // space that is purely rotation / translation from device space (and scale from src space).
        // We'll meet in the middle: pre-scale the src rect to be in this space and then apply the
        // inverse of the rotation / translation portion to the frag coord.
        SkMatrix m;
        SkSize scale;
        if (!localToDevice.decomposeScale(&scale, &m)) {
            return std::nullopt;
        }
        if (!m.invert(&devToScaledShape)) {
            return std::nullopt;
        }
        devRect = {srcRect.left() * scale.width(),
                   srcRect.top() * scale.height(),
                   srcRect.right() * scale.width(),
                   srcRect.bottom() * scale.height()};
    }

    if (!recorder->priv().caps()->shaderCaps()->fFloatIs32Bits) {
        // We promote the math that gets us into the Gaussian space to full float when the rect
        // coords are large. If we don't have full float then fail. We could probably clip the rect
        // to an outset device bounds instead.
        if (std::fabs(devRect.left()) > 16000.0f || std::fabs(devRect.top()) > 16000.0f ||
            std::fabs(devRect.right()) > 16000.0f || std::fabs(devRect.bottom()) > 16000.0f) {
            return std::nullopt;
        }
    }

    const float sixSigma = 6.0f * devSigma;
    const int tableWidth = ComputeIntegralTableWidth(sixSigma);
    UniqueKey key;
    {
        static const UniqueKey::Domain kRectBlurDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kRectBlurDomain, 1, "BlurredRectIntegralTable");
        builder[0] = tableWidth;
    }
    sk_sp<TextureProxy> integral = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, &tableWidth,
            [](const void* context) {
                int tableWidth = *static_cast<const int*>(context);
                return CreateIntegralTable(tableWidth);
            });

    if (!integral) {
        return std::nullopt;
    }

    // In the fast variant we think of the midpoint of the integral texture as aligning with the
    // closest rect edge both in x and y. To simplify texture coord calculation we inset the rect so
    // that the edge of the inset rect corresponds to t = 0 in the texture. It actually simplifies
    // things a bit in the !isFast case, too.
    const float threeSigma = 3.0f * devSigma;
    const Rect shapeData = Rect(devRect.left() + threeSigma,
                                devRect.top() + threeSigma,
                                devRect.right() - threeSigma,
                                devRect.bottom() - threeSigma);

    // In our fast variant we find the nearest horizontal and vertical edges and for each do a
    // lookup in the integral texture for each and multiply them. When the rect is less than 6*sigma
    // wide then things aren't so simple and we have to consider both the left and right edge of the
    // rectangle (and similar in y).
    const bool isFast = shapeData.left() <= shapeData.right() && shapeData.top() <= shapeData.bot();

    const float invSixSigma = 1.0f / sixSigma;

    // Determine how much to outset the draw bounds to ensure we hit pixels within 3*sigma.
    std::optional<Rect> drawBounds = outset_bounds(localToDevice, devSigma, srcRect);
    if (!drawBounds) {
        return std::nullopt;
    }

    return AnalyticBlurMask(*drawBounds,
                            SkM44(devToScaledShape),
                            ShapeType::kRect,
                            shapeData,
                            {static_cast<float>(isFast), invSixSigma},
                            integral);
}

static float quantize(float deviceSpaceFloat) {
    // Snap the device-space value to the nearest 1/32 to increase cache hits w/o impacting the
    // visible output since it should be hard to see a change limited to 1/32 of a pixel.
    // Clamp the value to 1/32 as identity blurs and points should be caught earlier.
    return std::max(SkScalarRoundToInt(deviceSpaceFloat * 32.f) / 32.f, 1.f / 32.f);
}

std::optional<AnalyticBlurMask> AnalyticBlurMask::MakeCircle(Recorder* recorder,
                                                             const SkMatrix& localToDevice,
                                                             float devSigma,
                                                             const SkRect& srcRect,
                                                             const SkRect& devRect) {
    const float radius = devRect.width() / 2.0f;
    if (!SkIsFinite(radius) || radius < SK_ScalarNearlyZero) {
        return std::nullopt;
    }

    // Pack profile-dependent properties and derived values into a struct that can be passed into
    // findOrCreateCachedProxy to lazily invoke the profile creation bitmap factories.
    struct DerivedParams {
        float fQuantizedRadius;
        float fQuantizedDevSigma;

        float fSolidRadius;
        float fTextureRadius;

        bool  fUseHalfPlaneApprox;

        DerivedParams(float devSigma, float radius)
                : fQuantizedRadius(quantize(radius))
                , fQuantizedDevSigma(quantize(devSigma)) {
            SkASSERT(fQuantizedRadius > 0.f); // quantization shouldn't have rounded to 0

            // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
            // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to
            // the Guassian and the profile texture is a just a Gaussian evaluation. However, we
            // haven't yet implemented this latter optimization.
            constexpr float kHalfPlaneThreshold = 0.1f;
            const float sigmaToRadiusRatio = std::min(fQuantizedDevSigma / fQuantizedRadius, 8.0f);
            if (sigmaToRadiusRatio <= kHalfPlaneThreshold) {
                fUseHalfPlaneApprox = true;
                fSolidRadius = fQuantizedRadius - 3.0f * fQuantizedDevSigma;
                fTextureRadius = 6.0f * fQuantizedDevSigma;
            } else {
                fUseHalfPlaneApprox = false;
                fQuantizedDevSigma = fQuantizedRadius * sigmaToRadiusRatio;
                fSolidRadius = 0.0f;
                fTextureRadius = fQuantizedRadius + 3.0f * fQuantizedDevSigma;
            }
        }
    } params{devSigma, radius};

    UniqueKey key;
    {
        static const UniqueKey::Domain kCircleBlurDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kCircleBlurDomain, 2, "BlurredCircleIntegralTable");
        if (params.fUseHalfPlaneApprox) {
            // There only ever needs to be one half plane approximation table, so store {0,0} into
            // the key, which never arises under normal use because we reject radius = 0 above.
            builder[0] = SkFloat2Bits(0.f);
            builder[1] = SkFloat2Bits(0.f);
        } else {
            builder[0] = SkFloat2Bits(params.fQuantizedDevSigma);
            builder[1] = SkFloat2Bits(params.fQuantizedRadius);
        }
    }
    sk_sp<TextureProxy> profile = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, &params,
            [](const void* context) {
                constexpr int kProfileTextureWidth = 512;
                const DerivedParams* params = static_cast<const DerivedParams*>(context);
                if (params->fUseHalfPlaneApprox) {
                    return CreateHalfPlaneProfile(kProfileTextureWidth);
                } else {
                    // Rescale params to the size of the texture we're creating.
                    const float scale = kProfileTextureWidth / params->fTextureRadius;
                    return CreateCircleProfile(params->fQuantizedDevSigma * scale,
                                               params->fQuantizedRadius * scale,
                                               kProfileTextureWidth);
                }
            });

    if (!profile) {
        return std::nullopt;
    }

    // In the shader we calculate an index into the blur profile
    // "i = (length(fragCoords - circleCenter) - solidRadius + 0.5) / textureRadius" as
    // "i = length((fragCoords - circleCenter) / textureRadius) -
    //      (solidRadius - 0.5) / textureRadius"
    // to avoid passing large values to length() that would overflow. We precalculate
    // "1 / textureRadius" and "(solidRadius - 0.5) / textureRadius" here.
    const Rect shapeData = Rect(devRect.centerX(),
                                devRect.centerY(),
                                1.0f / params.fTextureRadius,
                                (params.fSolidRadius - 0.5f) / params.fTextureRadius);

    // Determine how much to outset the draw bounds to ensure we hit pixels within 3*sigma.
    std::optional<Rect> drawBounds = outset_bounds(localToDevice,
                                                   params.fQuantizedDevSigma,
                                                   srcRect);
    if (!drawBounds) {
        return std::nullopt;
    }

    constexpr float kUnusedBlurData = 0.0f;
    return AnalyticBlurMask(*drawBounds,
                            SkM44(),
                            ShapeType::kCircle,
                            shapeData,
                            {kUnusedBlurData, kUnusedBlurData},
                            profile);
}

std::optional<AnalyticBlurMask> AnalyticBlurMask::MakeRRect(Recorder* recorder,
                                                            const SkMatrix& localToDevice,
                                                            float devSigma,
                                                            const SkRRect& srcRRect,
                                                            const SkRRect& devRRect) {
    const int devBlurRadius = 3 * SkScalarCeilToInt(devSigma - 1.0f / 6.0f);

    const SkVector& devRadiiUL = devRRect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& devRadiiUR = devRRect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& devRadiiLR = devRRect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& devRadiiLL = devRRect.radii(SkRRect::kLowerLeft_Corner);

    const int devLeft = SkScalarCeilToInt(std::max<float>(devRadiiUL.fX, devRadiiLL.fX));
    const int devTop = SkScalarCeilToInt(std::max<float>(devRadiiUL.fY, devRadiiUR.fY));
    const int devRight = SkScalarCeilToInt(std::max<float>(devRadiiUR.fX, devRadiiLR.fX));
    const int devBot = SkScalarCeilToInt(std::max<float>(devRadiiLL.fY, devRadiiLR.fY));

    // This is a conservative check for nine-patchability.
    const SkRect& devOrig = devRRect.getBounds();
    if (devOrig.fLeft + devLeft + devBlurRadius >= devOrig.fRight - devRight - devBlurRadius ||
        devOrig.fTop + devTop + devBlurRadius >= devOrig.fBottom - devBot - devBlurRadius) {
        return std::nullopt;
    }

    const int newRRWidth = 2 * devBlurRadius + devLeft + devRight + 1;
    const int newRRHeight = 2 * devBlurRadius + devTop + devBot + 1;

    const SkRect newRect = SkRect::MakeXYWH(SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(newRRWidth),
                                            SkIntToScalar(newRRHeight));
    SkVector newRadii[4];
    newRadii[0] = {SkScalarCeilToScalar(devRadiiUL.fX), SkScalarCeilToScalar(devRadiiUL.fY)};
    newRadii[1] = {SkScalarCeilToScalar(devRadiiUR.fX), SkScalarCeilToScalar(devRadiiUR.fY)};
    newRadii[2] = {SkScalarCeilToScalar(devRadiiLR.fX), SkScalarCeilToScalar(devRadiiLR.fY)};
    newRadii[3] = {SkScalarCeilToScalar(devRadiiLL.fX), SkScalarCeilToScalar(devRadiiLL.fY)};

    // NOTE: SkRRect does not satisfy std::has_unique_object_representation because NaN's in float
    // values violate that, but all SkRRects that get here will be finite so it's not really a
    // an issue for hashing the data directly.
    SK_BEGIN_REQUIRE_DENSE
    struct DerivedParams {
        SkRRect fRRectToDraw;
        SkISize fDimensions;
        float   fDevSigma;
    } params;
    SK_END_REQUIRE_DENSE

    params.fRRectToDraw.setRectRadii(newRect, newRadii);
    params.fDimensions =
            SkISize::Make(newRRWidth + 2 * devBlurRadius, newRRHeight + 2 * devBlurRadius);
    params.fDevSigma = devSigma;

    // TODO(b/343684954, b/338032240): This is just generating a blurred rrect mask image on the CPU
    // and uploading it. We should either generate them on the GPU and cache them here, or if we
    // have a general-purpose blur mask cache, then there's no reason rrects couldn't just use that
    // since this "analytic" blur isn't actually simplifying work like the circle and rect case.
    // That would also allow us to support arbitrary blurred rrects and not just ninepatch rrects.
    static const UniqueKey::Domain kRRectBlurDomain = UniqueKey::GenerateDomain();
    UniqueKey key;
    {
        static constexpr int kKeySize = sizeof(DerivedParams) / sizeof(uint32_t);
        static_assert(SkIsAlign4(sizeof(DerivedParams)));
        // TODO: We should discretize the sigma to perceptibly meaningful changes to the table,
        // as well as the underlying the round rect geometry.
        UniqueKey::Builder builder(&key, kRRectBlurDomain, kKeySize, "BlurredRRectNinePatch");
        memcpy(&builder[0], &params, sizeof(DerivedParams));
    }
    sk_sp<TextureProxy> ninePatch = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, &params,
            [](const void* context) {
                const DerivedParams* params = static_cast<const DerivedParams*>(context);
                return CreateRRectBlurMask(params->fRRectToDraw,
                                           params->fDimensions,
                                           params->fDevSigma);
            });

    if (!ninePatch) {
        return std::nullopt;
    }

    const float blurRadius = 3.0f * SkScalarCeilToScalar(devSigma - 1.0f / 6.0f);
    const float edgeSize = 2.0f * blurRadius + SkRRectPriv::GetSimpleRadii(devRRect).fX + 0.5f;
    const Rect shapeData = devRRect.rect().makeOutset(blurRadius, blurRadius);

    // Determine how much to outset the draw bounds to ensure we hit pixels within 3*sigma.
    std::optional<Rect> drawBounds = outset_bounds(localToDevice, devSigma, srcRRect.rect());
    if (!drawBounds) {
        return std::nullopt;
    }

    constexpr float kUnusedBlurData = 0.0f;
    return AnalyticBlurMask(*drawBounds,
                            SkM44(),
                            ShapeType::kRRect,
                            shapeData,
                            {edgeSize, kUnusedBlurData},
                            ninePatch);
}

}  // namespace skgpu::graphite
