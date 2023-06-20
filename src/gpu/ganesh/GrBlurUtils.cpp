/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBlurUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatBits.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkBlurMaskFilterImpl.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFixedClip.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrThreadSafeCache.h"
#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

static constexpr auto kMaskOrigin = kTopLeft_GrSurfaceOrigin;

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(skgpu::ganesh::SurfaceDrawContext* sdc,
                      const GrClip* clip,
                      const SkMatrix& viewMatrix,
                      const SkIRect& maskBounds,
                      GrPaint&& paint,
                      GrSurfaceProxyView mask) {
    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }

    mask.concatSwizzle(skgpu::Swizzle("aaaa"));

    SkMatrix matrix = SkMatrix::Translate(-SkIntToScalar(maskBounds.fLeft),
                                          -SkIntToScalar(maskBounds.fTop));
    matrix.preConcat(viewMatrix);
    paint.setCoverageFragmentProcessor(
            GrTextureEffect::Make(std::move(mask), kUnknown_SkAlphaType, matrix));

    sdc->fillPixelsWithLocalMatrix(clip, std::move(paint), maskBounds, inverse);
    return true;
}

static void mask_release_proc(void* addr, void* /*context*/) {
    SkMask::FreeImage(addr);
}

// This stores the mapping from an unclipped, integerized, device-space, shape bounds to
// the filtered mask's draw rect.
struct DrawRectData {
    SkIVector fOffset;
    SkISize   fSize;
};

static sk_sp<SkData> create_data(const SkIRect& drawRect, const SkIRect& origDevBounds) {

    DrawRectData drawRectData { {drawRect.fLeft - origDevBounds.fLeft,
                                 drawRect.fTop - origDevBounds.fTop},
                                drawRect.size() };

    return SkData::MakeWithCopy(&drawRectData, sizeof(drawRectData));
}

static SkIRect extract_draw_rect_from_data(SkData* data, const SkIRect& origDevBounds) {
    auto drawRectData = static_cast<const DrawRectData*>(data->data());

    return SkIRect::MakeXYWH(origDevBounds.fLeft + drawRectData->fOffset.fX,
                             origDevBounds.fTop + drawRectData->fOffset.fY,
                             drawRectData->fSize.fWidth,
                             drawRectData->fSize.fHeight);
}

static GrSurfaceProxyView sw_create_filtered_mask(GrRecordingContext* rContext,
                                                  const SkMatrix& viewMatrix,
                                                  const GrStyledShape& shape,
                                                  const SkMaskFilter* filter,
                                                  const SkIRect& unclippedDevShapeBounds,
                                                  const SkIRect& clipBounds,
                                                  SkIRect* drawRect,
                                                  skgpu::UniqueKey* key) {
    SkASSERT(filter);
    SkASSERT(!shape.style().applies());

    auto threadSafeCache = rContext->priv().threadSafeCache();

    GrSurfaceProxyView filteredMaskView;
    sk_sp<SkData> data;

    if (key->isValid()) {
        std::tie(filteredMaskView, data) = threadSafeCache->findWithData(*key);
    }

    if (filteredMaskView) {
        SkASSERT(data);
        SkASSERT(kMaskOrigin == filteredMaskView.origin());

        *drawRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
    } else {
        SkStrokeRec::InitStyle fillOrHairline = shape.style().isSimpleHairline()
                                                        ? SkStrokeRec::kHairline_InitStyle
                                                        : SkStrokeRec::kFill_InitStyle;

        // TODO: it seems like we could create an SkDraw here and set its fMatrix field rather
        // than explicitly transforming the path to device space.
        SkPath devPath;

        shape.asPath(&devPath);

        devPath.transform(viewMatrix);

        SkMask srcM, dstM;
        if (!SkDraw::DrawToMask(devPath, clipBounds, filter, &viewMatrix, &srcM,
                                SkMask::kComputeBoundsAndRenderImage_CreateMode, fillOrHairline)) {
            return {};
        }
        SkAutoMaskFreeImage autoSrc(srcM.fImage);

        SkASSERT(SkMask::kA8_Format == srcM.fFormat);

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return {};
        }
        // this will free-up dstM when we're done (allocated in filterMask())
        SkAutoMaskFreeImage autoDst(dstM.fImage);

        if (clip_bounds_quick_reject(clipBounds, dstM.fBounds)) {
            return {};
        }

        // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
        // the current clip (and identity matrix) and GrPaint settings
        SkBitmap bm;
        if (!bm.installPixels(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                              autoDst.release(), dstM.fRowBytes, mask_release_proc, nullptr)) {
            return {};
        }
        bm.setImmutable();

        std::tie(filteredMaskView, std::ignore) = GrMakeUncachedBitmapProxyView(
                rContext,
                bm,
                GrMipmapped::kNo,
                SkBackingFit::kApprox);
        if (!filteredMaskView) {
            return {};
        }

        SkASSERT(kMaskOrigin == filteredMaskView.origin());

        *drawRect = dstM.fBounds;

        if (key->isValid()) {
            key->setCustomData(create_data(*drawRect, unclippedDevShapeBounds));
            std::tie(filteredMaskView, data) = threadSafeCache->addWithData(*key, filteredMaskView);
            // If we got a different view back from 'addWithData' it could have a different drawRect
            *drawRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
        }
    }

    return filteredMaskView;
}

// Create a mask of 'shape' and return the resulting surfaceDrawContext
static std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> create_mask_GPU(
        GrRecordingContext* rContext,
        const SkIRect& maskRect,
        const SkMatrix& origViewMatrix,
        const GrStyledShape& shape,
        int sampleCnt) {
    // We cache blur masks. Use default surface props here so we can use the same cached mask
    // regardless of the final dst surface.
    SkSurfaceProps defaultSurfaceProps;

    // Use GrResourceProvider::MakeApprox to implement our own approximate size matching, but demand
    // a "SkBackingFit::kExact" size match on the actual render target. We do this because the
    // filter will reach outside the src bounds, so we need to pre-clear these values to ensure a
    // "decal" sampling effect (i.e., ensure reads outside the src bounds return alpha=0).
    //
    // FIXME: Reads outside the left and top edges will actually clamp to the edge pixel. And in the
    // event that MakeApprox does not change the size, reads outside the right and/or bottom will do
    // the same. We should offset our filter within the render target and expand the size as needed
    // to guarantee at least 1px of padding on all sides.
    auto approxSize = GrResourceProvider::MakeApprox(maskRect.size());
    auto sdc = skgpu::ganesh::SurfaceDrawContext::MakeWithFallback(rContext,
                                                                   GrColorType::kAlpha_8,
                                                                   nullptr,
                                                                   SkBackingFit::kExact,
                                                                   approxSize,
                                                                   defaultSurfaceProps,
                                                                   sampleCnt,
                                                                   GrMipmapped::kNo,
                                                                   GrProtected::kNo,
                                                                   kMaskOrigin);
    if (!sdc) {
        return nullptr;
    }

    sdc->clear(SK_PMColor4fTRANSPARENT);

    GrPaint maskPaint;
    maskPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    GrFixedClip clip(sdc->dimensions(), SkIRect::MakeWH(maskRect.width(), maskRect.height()));

    // Draw the mask into maskTexture with the path's integerized top-left at the origin using
    // maskPaint.
    SkMatrix viewMatrix = origViewMatrix;
    viewMatrix.postTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    sdc->drawShape(&clip, std::move(maskPaint), GrAA::kYes, viewMatrix, GrStyledShape(shape));
    return sdc;
}

static bool get_unclipped_shape_dev_bounds(const GrStyledShape& shape, const SkMatrix& matrix,
                                           SkIRect* devBounds) {
    SkRect shapeDevBounds;
    if (shape.inverseFilled()) {
        shapeDevBounds = {SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity,
                          SK_ScalarInfinity, SK_ScalarInfinity};
    } else {
        SkRect shapeBounds = shape.styledBounds();
        if (shapeBounds.isEmpty()) {
            return false;
        }
        matrix.mapRect(&shapeDevBounds, shapeBounds);
    }
    // Even though these are "unclipped" bounds we still clip to the int32_t range.
    // This is the largest int32_t that is representable exactly as a float. The next 63 larger ints
    // would round down to this value when cast to a float, but who really cares.
    // INT32_MIN is exactly representable.
    static constexpr int32_t kMaxInt = 2147483520;
    if (!shapeDevBounds.intersect(SkRect::MakeLTRB(INT32_MIN, INT32_MIN, kMaxInt, kMaxInt))) {
        return false;
    }
    // Make sure that the resulting SkIRect can have representable width and height
    if (SkScalarRoundToInt(shapeDevBounds.width()) > kMaxInt ||
        SkScalarRoundToInt(shapeDevBounds.height()) > kMaxInt) {
        return false;
    }
    shapeDevBounds.roundOut(devBounds);
    return true;
}

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
static bool get_shape_and_clip_bounds(skgpu::ganesh::SurfaceDrawContext* sdc,
                                      const GrClip* clip,
                                      const GrStyledShape& shape,
                                      const SkMatrix& matrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    *devClipBounds = clip ? clip->getConservativeBounds()
                          : SkIRect::MakeWH(sdc->width(), sdc->height());

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::MakeEmpty();
        return false;
    }

    return true;
}

/**
 *  If we cannot create a FragmentProcess for a mask filter, we might have special logic for
 *  it here. That code path requires constructing a src mask as input. Since that is a potentially
 *  expensive operation, this function tests if filter_mask would succeed if the mask
 *  were to be created.
 *
 *  'maskRect' returns the device space portion of the mask that the filter needs. The mask
 *  passed into 'filter_mask' should have the same extent as 'maskRect' but be
 *  translated to the upper-left corner of the mask (i.e., (maskRect.fLeft, maskRect.fTop)
 *  appears at (0, 0) in the mask).
 *
 * Logically, how this works is:
 *    can_filter_mask is called
 *    if (it returns true)
 *        the returned mask rect is used for quick rejecting
 *            the mask rect is used to generate the mask
 *            filter_mask is called to filter the mask
 *
 * TODO: this should work as:
 *    if (can_filter_mask(devShape, ...)) // rect, rrect, drrect, path
 *        filter_mask(devShape, ...)
 * this would hide the RRect special case and the mask generation
 */
static bool can_filter_mask(const SkMaskFilterBase* maskFilter,
                            const GrStyledShape& shape,
                            const SkIRect& devSpaceShapeBounds,
                            const SkIRect& clipBounds,
                            const SkMatrix& ctm,
                            SkIRect* maskRect) {
    if (maskFilter->type() != SkMaskFilterBase::Type::kBlur) {
        return false;
    }
    auto bmf = static_cast<const SkBlurMaskFilterImpl*>(maskFilter);
    SkScalar xformedSigma = bmf->computeXformedSigma(ctm);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        *maskRect = devSpaceShapeBounds;
        return maskRect->intersect(clipBounds);
    }

    if (maskRect) {
        float sigma3 = 3 * SkScalarToFloat(xformedSigma);

        // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
        SkIRect clipRect = clipBounds.makeOutset(sigma3, sigma3);
        SkIRect srcRect = devSpaceShapeBounds.makeOutset(sigma3, sigma3);

        if (!srcRect.intersect(clipRect)) {
            srcRect.setEmpty();
        }
        *maskRect = srcRect;
    }

    // We prefer to blur paths with small blur radii on the CPU.
    static const SkScalar kMIN_GPU_BLUR_SIZE  = SkIntToScalar(64);
    static const SkScalar kMIN_GPU_BLUR_SIGMA = SkIntToScalar(32);

    if (devSpaceShapeBounds.width() <= kMIN_GPU_BLUR_SIZE &&
        devSpaceShapeBounds.height() <= kMIN_GPU_BLUR_SIZE &&
        xformedSigma <= kMIN_GPU_BLUR_SIGMA) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  Circle Blur
///////////////////////////////////////////////////////////////////////////////

// Computes an unnormalized half kernel (right side). Returns the summation of all the half
// kernel values.
static float make_unnormalized_half_kernel(float* halfKernel, int halfKernelSize, float sigma) {
    const float invSigma = 1.f / sigma;
    const float b = -0.5f * invSigma * invSigma;
    float tot = 0.0f;
    // Compute half kernel values at half pixel steps out from the center.
    float t = 0.5f;
    for (int i = 0; i < halfKernelSize; ++i) {
        float value = expf(t * t * b);
        tot += value;
        halfKernel[i] = value;
        t += 1.f;
    }
    return tot;
}

// Create a Gaussian half-kernel (right side) and a summed area table given a sigma and number
// of discrete steps. The half kernel is normalized to sum to 0.5.
static void make_half_kernel_and_summed_table(float* halfKernel,
                                              float* summedHalfKernel,
                                              int halfKernelSize,
                                              float sigma) {
    // The half kernel should sum to 0.5 not 1.0.
    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel, halfKernelSize, sigma);
    float sum = 0.f;
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[i] /= tot;
        sum += halfKernel[i];
        summedHalfKernel[i] = sum;
    }
}

// Applies the 1D half kernel vertically at points along the x axis to a circle centered at the
// origin with radius circleR.
static void apply_kernel_in_y(float* results,
                              int numSteps,
                              float firstX,
                              float circleR,
                              int halfKernelSize,
                              const float* summedHalfKernelTable) {
    float x = firstX;
    for (int i = 0; i < numSteps; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            results[i] = 0;
            continue;
        }
        float y = sqrtf(circleR * circleR - x * x);
        // In the column at x we exit the circle at +y and -y
        // The summed table entry j is actually reflects an offset of j + 0.5.
        y -= 0.5f;
        int yInt = SkScalarFloorToInt(y);
        SkASSERT(yInt >= -1);
        if (y < 0) {
            results[i] = (y + 0.5f) * summedHalfKernelTable[0];
        } else if (yInt >= halfKernelSize - 1) {
            results[i] = 0.5f;
        } else {
            float yFrac = y - yInt;
            results[i] = (1.f - yFrac) * summedHalfKernelTable[yInt] +
                         yFrac * summedHalfKernelTable[yInt + 1];
        }
    }
}

// Apply a Gaussian at point (evalX, 0) to a circle centered at the origin with radius circleR.
// This relies on having a half kernel computed for the Gaussian and a table of applications of
// the half kernel in y to columns at (evalX - halfKernel, evalX - halfKernel + 1, ..., evalX +
// halfKernel) passed in as yKernelEvaluations.
static uint8_t eval_at(float evalX,
                       float circleR,
                       const float* halfKernel,
                       int halfKernelSize,
                       const float* yKernelEvaluations) {
    float acc = 0;

    float x = evalX - halfKernelSize;
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i];
        acc += verticalEval * halfKernel[halfKernelSize - i - 1];
    }
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i + halfKernelSize];
        acc += verticalEval * halfKernel[i];
    }
    // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about
    // the x axis).
    return SkUnitScalarClampToByte(2.f * acc);
}

// This function creates a profile of a blurred circle. It does this by computing a kernel for
// half the Gaussian and a matching summed area table. The summed area table is used to compute
// an array of vertical applications of the half kernel to the circle along the x axis. The
// table of y evaluations has 2 * k + n entries where k is the size of the half kernel and n is
// the size of the profile being computed. Then for each of the n profile entries we walk out k
// steps in each horizontal direction multiplying the corresponding y evaluation by the half
// kernel entry and sum these values to compute the profile entry.
static void create_circle_profile(uint8_t* weights,
                                  float sigma,
                                  float circleR,
                                  int profileTextureWidth) {
    const int numSteps = profileTextureWidth;

    // The full kernel is 6 sigmas wide.
    int halfKernelSize = SkScalarCeilToInt(6.0f * sigma);
    // round up to next multiple of 2 and then divide by 2
    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

    // Number of x steps at which to apply kernel in y to cover all the profile samples in x.
    int numYSteps = numSteps + 2 * halfKernelSize;

    skia_private::AutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
    float* halfKernel = bulkAlloc.get();
    float* summedKernel = bulkAlloc.get() + halfKernelSize;
    float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
    make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

    float firstX = -halfKernelSize + 0.5f;
    apply_kernel_in_y(yEvals, numYSteps, firstX, circleR, halfKernelSize, summedKernel);

    for (int i = 0; i < numSteps - 1; ++i) {
        float evalX = i + 0.5f;
        weights[i] = eval_at(evalX, circleR, halfKernel, halfKernelSize, yEvals + i);
    }
    // Ensure the tail of the Gaussian goes to zero.
    weights[numSteps - 1] = 0;
}

static void create_half_plane_profile(uint8_t* profile, int profileWidth) {
    SkASSERT(!(profileWidth & 0x1));
    // The full kernel is 6 sigmas wide.
    float sigma = profileWidth / 6.f;
    int halfKernelSize = profileWidth / 2;

    skia_private::AutoTArray<float> halfKernel(halfKernelSize);

    // The half kernel should sum to 0.5.
    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel.get(), halfKernelSize, sigma);
    float sum = 0.f;
    // Populate the profile from the right edge to the middle.
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[halfKernelSize - i - 1] /= tot;
        sum += halfKernel[halfKernelSize - i - 1];
        profile[profileWidth - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Populate the profile from the middle to the left edge (by flipping the half kernel and
    // continuing the summation).
    for (int i = 0; i < halfKernelSize; ++i) {
        sum += halfKernel[i];
        profile[halfKernelSize - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Ensure tail goes to 0.
    profile[profileWidth - 1] = 0;
}

static std::unique_ptr<GrFragmentProcessor> create_profile_effect(GrRecordingContext* rContext,
                                                                  const SkRect& circle,
                                                                  float sigma,
                                                                  float* solidRadius,
                                                                  float* textureRadius) {
    float circleR = circle.width() / 2.0f;
    if (!sk_float_isfinite(circleR) || circleR < SK_ScalarNearlyZero) {
        return nullptr;
    }

    auto threadSafeCache = rContext->priv().threadSafeCache();

    // Profile textures are cached by the ratio of sigma to circle radius and by the size of the
    // profile texture (binned by powers of 2).
    SkScalar sigmaToCircleRRatio = sigma / circleR;
    // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
    // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to the
    // Guassian and the profile texture is a just a Gaussian evaluation. However, we haven't yet
    // implemented this latter optimization.
    sigmaToCircleRRatio = std::min(sigmaToCircleRRatio, 8.f);
    SkFixed sigmaToCircleRRatioFixed;
    static const SkScalar kHalfPlaneThreshold = 0.1f;
    bool useHalfPlaneApprox = false;
    if (sigmaToCircleRRatio <= kHalfPlaneThreshold) {
        useHalfPlaneApprox = true;
        sigmaToCircleRRatioFixed = 0;
        *solidRadius = circleR - 3 * sigma;
        *textureRadius = 6 * sigma;
    } else {
        // Convert to fixed point for the key.
        sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);
        // We shave off some bits to reduce the number of unique entries. We could probably
        // shave off more than we do.
        sigmaToCircleRRatioFixed &= ~0xff;
        sigmaToCircleRRatio = SkFixedToScalar(sigmaToCircleRRatioFixed);
        sigma = circleR * sigmaToCircleRRatio;
        *solidRadius = 0;
        *textureRadius = circleR + 3 * sigma;
    }

    static constexpr int kProfileTextureWidth = 512;
    // This would be kProfileTextureWidth/textureRadius if it weren't for the fact that we do
    // the calculation of the profile coord in a coord space that has already been scaled by
    // 1 / textureRadius. This is done to avoid overflow in length().
    SkMatrix texM = SkMatrix::Scale(kProfileTextureWidth, 1.f);

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    skgpu::UniqueKey::Builder builder(&key, kDomain, 1, "1-D Circular Blur");
    builder[0] = sigmaToCircleRRatioFixed;
    builder.finish();

    GrSurfaceProxyView profileView = threadSafeCache->find(key);
    if (profileView) {
        SkASSERT(profileView.asTextureProxy());
        SkASSERT(profileView.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeA8(kProfileTextureWidth, 1))) {
        return nullptr;
    }

    if (useHalfPlaneApprox) {
        create_half_plane_profile(bm.getAddr8(0, 0), kProfileTextureWidth);
    } else {
        // Rescale params to the size of the texture we're creating.
        SkScalar scale = kProfileTextureWidth / *textureRadius;
        create_circle_profile(
                bm.getAddr8(0, 0), sigma * scale, circleR * scale, kProfileTextureWidth);
    }
    bm.setImmutable();

    profileView = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bm));
    if (!profileView) {
        return nullptr;
    }

    profileView = threadSafeCache->add(key, profileView);
    return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
}

static std::unique_ptr<GrFragmentProcessor> make_circle_blur(GrRecordingContext* context,
                                                             const SkRect& circle,
                                                             float sigma) {
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma)) {
        return nullptr;
    }

    float solidRadius;
    float textureRadius;
    std::unique_ptr<GrFragmentProcessor> profile =
            create_profile_effect(context, circle, sigma, &solidRadius, &textureRadius);
    if (!profile) {
        return nullptr;
    }

    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader blurProfile;"
        "uniform half4 circleData;"

        "half4 main(float2 xy) {"
            // We just want to compute "(length(vec) - circleData.z + 0.5) * circleData.w" but need
            // to rearrange to avoid passing large values to length() that would overflow.
            "half2 vec = half2((sk_FragCoord.xy - circleData.xy) * circleData.w);"
            "half dist = length(vec) + (0.5 - circleData.z) * circleData.w;"
            "return blurProfile.eval(half2(dist, 0.5)).aaaa;"
        "}"
    );

    SkV4 circleData = {circle.centerX(), circle.centerY(), solidRadius, 1.f / textureRadius};
    auto circleBlurFP = GrSkSLFP::Make(effect, "CircleBlur", /*inputFP=*/nullptr,
                                       GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                                       "blurProfile", GrSkSLFP::IgnoreOptFlags(std::move(profile)),
                                       "circleData", circleData);
    // Modulate blur with the input color.
    return GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(circleBlurFP),
                                                                  /*dst=*/nullptr);
}

///////////////////////////////////////////////////////////////////////////////
//  Rect Blur
///////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<GrFragmentProcessor> make_rect_integral_fp(GrRecordingContext* rContext,
                                                                  float sixSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sixSigma / 6.f));
    auto threadSafeCache = rContext->priv().threadSafeCache();

    int width = SkGpuBlurUtils::CreateIntegralTable(sixSigma, nullptr);

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    skgpu::UniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
    builder[0] = width;
    builder.finish();

    SkMatrix m = SkMatrix::Scale(width / sixSigma, 1.f);

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

    view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap));
    if (!view) {
        return {};
    }

    view = threadSafeCache->add(key, view);

    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    return GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
}

static std::unique_ptr<GrFragmentProcessor> make_rect_blur(GrRecordingContext* context,
                                                           const GrShaderCaps& caps,
                                                           const SkRect& srcRect,
                                                           const SkMatrix& viewMatrix,
                                                           float transformedSigma) {
    SkASSERT(viewMatrix.preservesRightAngles());
    SkASSERT(srcRect.isSorted());

    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(transformedSigma)) {
        // No need to blur the rect
        return nullptr;
    }

    SkMatrix invM;
    SkRect rect;
    if (viewMatrix.rectStaysRect()) {
        invM = SkMatrix::I();
        // We can do everything in device space when the src rect projects to a rect in device space
        SkAssertResult(viewMatrix.mapRect(&rect, srcRect));
    } else {
        // The view matrix may scale, perhaps anisotropically. But we want to apply our device space
        // "transformedSigma" to the delta of frag coord from the rect edges. Factor out the scaling
        // to define a space that is purely rotation/translation from device space (and scale from
        // src space) We'll meet in the middle: pre-scale the src rect to be in this space and then
        // apply the inverse of the rotation/translation portion to the frag coord.
        SkMatrix m;
        SkSize scale;
        if (!viewMatrix.decomposeScale(&scale, &m)) {
            return nullptr;
        }
        if (!m.invert(&invM)) {
            return nullptr;
        }
        rect = {srcRect.left() * scale.width(),
                srcRect.top() * scale.height(),
                srcRect.right() * scale.width(),
                srcRect.bottom() * scale.height()};
    }

    if (!caps.fFloatIs32Bits) {
        // We promote the math that gets us into the Gaussian space to full float when the rect
        // coords are large. If we don't have full float then fail. We could probably clip the rect
        // to an outset device bounds instead.
        if (SkScalarAbs(rect.fLeft) > 16000.f || SkScalarAbs(rect.fTop) > 16000.f ||
            SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f) {
            return nullptr;
        }
    }

    const float sixSigma = 6 * transformedSigma;
    std::unique_ptr<GrFragmentProcessor> integral = make_rect_integral_fp(context, sixSigma);
    if (!integral) {
        return nullptr;
    }

    // In the fast variant we think of the midpoint of the integral texture as aligning with the
    // closest rect edge both in x and y. To simplify texture coord calculation we inset the rect so
    // that the edge of the inset rect corresponds to t = 0 in the texture. It actually simplifies
    // things a bit in the !isFast case, too.
    float threeSigma = sixSigma / 2;
    SkRect insetRect = {rect.left() + threeSigma,
                        rect.top() + threeSigma,
                        rect.right() - threeSigma,
                        rect.bottom() - threeSigma};

    // In our fast variant we find the nearest horizontal and vertical edges and for each do a
    // lookup in the integral texture for each and multiply them. When the rect is less than 6 sigma
    // wide then things aren't so simple and we have to consider both the left and right edge of the
    // rectangle (and similar in y).
    bool isFast = insetRect.isSorted();

    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        // Effect that is a LUT for integral of normal distribution. The value at x:[0,6*sigma] is
        // the integral from -inf to (3*sigma - x). I.e. x is mapped from [0, 6*sigma] to
        // [3*sigma to -3*sigma]. The flip saves a reversal in the shader.
        "uniform shader integral;"

        "uniform float4 rect;"
        "uniform int isFast;"  // specialized

        "half4 main(float2 pos) {"
            "half xCoverage, yCoverage;"
            "if (bool(isFast)) {"
                // Get the smaller of the signed distance from the frag coord to the left and right
                // edges and similar for y.
                // The integral texture goes "backwards" (from 3*sigma to -3*sigma), So, the below
                // computations align the left edge of the integral texture with the inset rect's
                // edge extending outward 6 * sigma from the inset rect.
                "half2 xy = max(half2(rect.LT - pos), half2(pos - rect.RB));"
                "xCoverage = integral.eval(half2(xy.x, 0.5)).a;"
                "yCoverage = integral.eval(half2(xy.y, 0.5)).a;"
            "} else {"
                // We just consider just the x direction here. In practice we compute x and y
                // separately and multiply them together.
                // We define our coord system so that the point at which we're evaluating a kernel
                // defined by the normal distribution (K) at 0. In this coord system let L be left
                // edge and R be the right edge of the rectangle.
                // We can calculate C by integrating K with the half infinite ranges outside the
                // L to R range and subtracting from 1:
                //   C = 1 - <integral of K from from -inf to  L> - <integral of K from R to inf>
                // K is symmetric about x=0 so:
                //   C = 1 - <integral of K from from -inf to  L> - <integral of K from -inf to -R>

                // The integral texture goes "backwards" (from 3*sigma to -3*sigma) which is
                // factored in to the below calculations.
                // Also, our rect uniform was pre-inset by 3 sigma from the actual rect being
                // blurred, also factored in.
                "half4 rect = half4(half2(rect.LT - pos), half2(pos - rect.RB));"
                "xCoverage = 1 - integral.eval(half2(rect.L, 0.5)).a"
                              "- integral.eval(half2(rect.R, 0.5)).a;"
                "yCoverage = 1 - integral.eval(half2(rect.T, 0.5)).a"
                              "- integral.eval(half2(rect.B, 0.5)).a;"
            "}"
            "return half4(xCoverage * yCoverage);"
        "}"
    );

    std::unique_ptr<GrFragmentProcessor> fp =
            GrSkSLFP::Make(effect, "RectBlur", /*inputFP=*/nullptr,
                           GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                           "integral", GrSkSLFP::IgnoreOptFlags(std::move(integral)),
                           "rect", insetRect,
                           "isFast", GrSkSLFP::Specialize<int>(isFast));
    // Modulate blur with the input color.
    fp = GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(fp),
                                                                /*dst=*/nullptr);
    if (!invM.isIdentity()) {
        fp = GrMatrixEffect::Make(invM, std::move(fp));
    }
    return GrFragmentProcessor::DeviceSpace(std::move(fp));
}

///////////////////////////////////////////////////////////////////////////////
//  RRect Blur
///////////////////////////////////////////////////////////////////////////////

static constexpr auto kBlurredRRectMaskOrigin = kTopLeft_GrSurfaceOrigin;

static void make_blurred_rrect_key(skgpu::UniqueKey* key,
                                   const SkRRect& rrectToDraw,
                                   float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();

    skgpu::UniqueKey::Builder builder(key, kDomain, 9, "RoundRect Blur Mask");
    builder[0] = SkScalarCeilToInt(xformedSigma - 1 / 6.0f);

    int index = 1;
    // TODO: this is overkill for _simple_ circular rrects
    for (auto c : {SkRRect::kUpperLeft_Corner,
                   SkRRect::kUpperRight_Corner,
                   SkRRect::kLowerRight_Corner,
                   SkRRect::kLowerLeft_Corner}) {
        SkASSERT(SkScalarIsInt(rrectToDraw.radii(c).fX) && SkScalarIsInt(rrectToDraw.radii(c).fY));
        builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fX);
        builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fY);
    }
    builder.finish();
}

static bool fillin_view_on_gpu(GrDirectContext* dContext,
                               const GrSurfaceProxyView& lazyView,
                               sk_sp<GrThreadSafeCache::Trampoline> trampoline,
                               const SkRRect& rrectToDraw,
                               const SkISize& dimensions,
                               float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));

    // We cache blur masks. Use default surface props here so we can use the same cached mask
    // regardless of the final dst surface.
    SkSurfaceProps defaultSurfaceProps;

    std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> sdc =
            skgpu::ganesh::SurfaceDrawContext::MakeWithFallback(dContext,
                                                                GrColorType::kAlpha_8,
                                                                nullptr,
                                                                SkBackingFit::kExact,
                                                                dimensions,
                                                                defaultSurfaceProps,
                                                                1,
                                                                GrMipmapped::kNo,
                                                                GrProtected::kNo,
                                                                kBlurredRRectMaskOrigin);
    if (!sdc) {
        return false;
    }

    GrPaint paint;

    sdc->clear(SK_PMColor4fTRANSPARENT);
    sdc->drawRRect(nullptr,
                   std::move(paint),
                   GrAA::kYes,
                   SkMatrix::I(),
                   rrectToDraw,
                   GrStyle::SimpleFill());

    GrSurfaceProxyView srcView = sdc->readSurfaceView();
    SkASSERT(srcView.asTextureProxy());
    auto rtc2 = SkGpuBlurUtils::GaussianBlur(dContext,
                                             std::move(srcView),
                                             sdc->colorInfo().colorType(),
                                             sdc->colorInfo().alphaType(),
                                             nullptr,
                                             SkIRect::MakeSize(dimensions),
                                             SkIRect::MakeSize(dimensions),
                                             xformedSigma,
                                             xformedSigma,
                                             SkTileMode::kClamp,
                                             SkBackingFit::kExact);
    if (!rtc2 || !rtc2->readSurfaceView()) {
        return false;
    }

    auto view = rtc2->readSurfaceView();
    SkASSERT(view.swizzle() == lazyView.swizzle());
    SkASSERT(view.origin() == lazyView.origin());
    trampoline->fProxy = view.asTextureProxyRef();

    return true;
}

// Evaluate the vertical blur at the specified 'y' value given the location of the top of the
// rrect.
static uint8_t eval_V(float top, int y, const uint8_t* integral, int integralSize, float sixSigma) {
    if (top < 0) {
        return 0;  // an empty column
    }

    float fT = (top - y - 0.5f) * (integralSize / sixSigma);
    if (fT < 0) {
        return 255;
    } else if (fT >= integralSize - 1) {
        return 0;
    }

    int lower = (int)fT;
    float frac = fT - lower;

    SkASSERT(lower + 1 < integralSize);

    return integral[lower] * (1.0f - frac) + integral[lower + 1] * frac;
}

// Apply a gaussian 'kernel' horizontally at the specified 'x', 'y' location.
static uint8_t eval_H(int x,
                      int y,
                      const std::vector<float>& topVec,
                      const float* kernel,
                      int kernelSize,
                      const uint8_t* integral,
                      int integralSize,
                      float sixSigma) {
    SkASSERT(0 <= x && x < (int)topVec.size());
    SkASSERT(kernelSize % 2);

    float accum = 0.0f;

    int xSampleLoc = x - (kernelSize / 2);
    for (int i = 0; i < kernelSize; ++i, ++xSampleLoc) {
        if (xSampleLoc < 0 || xSampleLoc >= (int)topVec.size()) {
            continue;
        }

        accum += kernel[i] * eval_V(topVec[xSampleLoc], y, integral, integralSize, sixSigma);
    }

    return accum + 0.5f;
}

// Create a cpu-side blurred-rrect mask that is close to the version the gpu would've produced.
// The match needs to be close bc the cpu- and gpu-generated version must be interchangeable.
static GrSurfaceProxyView create_mask_on_cpu(GrRecordingContext* rContext,
                                             const SkRRect& rrectToDraw,
                                             const SkISize& dimensions,
                                             float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    int radius = SkGpuBlurUtils::SigmaRadius(xformedSigma);
    int kernelSize = 2 * radius + 1;

    SkASSERT(kernelSize % 2);
    SkASSERT(dimensions.width() % 2);
    SkASSERT(dimensions.height() % 2);

    SkVector radii = rrectToDraw.getSimpleRadii();
    SkASSERT(SkScalarNearlyEqual(radii.fX, radii.fY));

    const int halfWidthPlus1 = (dimensions.width() / 2) + 1;
    const int halfHeightPlus1 = (dimensions.height() / 2) + 1;

    std::unique_ptr<float[]> kernel(new float[kernelSize]);

    SkGpuBlurUtils::Compute1DGaussianKernel(kernel.get(), xformedSigma, radius);

    SkBitmap integral;
    if (!SkGpuBlurUtils::CreateIntegralTable(6 * xformedSigma, &integral)) {
        return {};
    }

    SkBitmap result;
    if (!result.tryAllocPixels(SkImageInfo::MakeA8(dimensions.width(), dimensions.height()))) {
        return {};
    }

    std::vector<float> topVec;
    topVec.reserve(dimensions.width());
    for (int x = 0; x < dimensions.width(); ++x) {
        if (x < rrectToDraw.rect().fLeft || x > rrectToDraw.rect().fRight) {
            topVec.push_back(-1);
        } else {
            if (x + 0.5f < rrectToDraw.rect().fLeft + radii.fX) {  // in the circular section
                float xDist = rrectToDraw.rect().fLeft + radii.fX - x - 0.5f;
                float h = sqrtf(radii.fX * radii.fX - xDist * xDist);
                SkASSERT(0 <= h && h < radii.fY);
                topVec.push_back(rrectToDraw.rect().fTop + radii.fX - h + 3 * xformedSigma);
            } else {
                topVec.push_back(rrectToDraw.rect().fTop + 3 * xformedSigma);
            }
        }
    }

    for (int y = 0; y < halfHeightPlus1; ++y) {
        uint8_t* scanline = result.getAddr8(0, y);

        for (int x = 0; x < halfWidthPlus1; ++x) {
            scanline[x] = eval_H(x,
                                 y,
                                 topVec,
                                 kernel.get(),
                                 kernelSize,
                                 integral.getAddr8(0, 0),
                                 integral.width(),
                                 6 * xformedSigma);
            scanline[dimensions.width() - x - 1] = scanline[x];
        }

        memcpy(result.getAddr8(0, dimensions.height() - y - 1), scanline, result.rowBytes());
    }

    result.setImmutable();

    auto view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, result));
    if (!view) {
        return {};
    }

    SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
    return view;
}

static std::unique_ptr<GrFragmentProcessor> find_or_create_rrect_blur_mask_fp(
        GrRecordingContext* rContext,
        const SkRRect& rrectToDraw,
        const SkISize& dimensions,
        float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    skgpu::UniqueKey key;
    make_blurred_rrect_key(&key, rrectToDraw, xformedSigma);

    auto threadSafeCache = rContext->priv().threadSafeCache();

    // It seems like we could omit this matrix and modify the shader code to not normalize
    // the coords used to sample the texture effect. However, the "proxyDims" value in the
    // shader is not always the actual the proxy dimensions. This is because 'dimensions' here
    // was computed using integer corner radii as determined in
    // SkComputeBlurredRRectParams whereas the shader code uses the float radius to compute
    // 'proxyDims'. Why it draws correctly with these unequal values is a mystery for the ages.
    auto m = SkMatrix::Scale(dimensions.width(), dimensions.height());

    GrSurfaceProxyView view;

    if (GrDirectContext* dContext = rContext->asDirectContext()) {
        // The gpu thread gets priority over the recording threads. If the gpu thread is first,
        // it crams a lazy proxy into the cache and then fills it in later.
        auto [lazyView, trampoline] = GrThreadSafeCache::CreateLazyView(dContext,
                                                                        GrColorType::kAlpha_8,
                                                                        dimensions,
                                                                        kBlurredRRectMaskOrigin,
                                                                        SkBackingFit::kExact);
        if (!lazyView) {
            return nullptr;
        }

        view = threadSafeCache->findOrAdd(key, lazyView);
        if (view != lazyView) {
            SkASSERT(view.asTextureProxy());
            SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
            return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
        }

        if (!fillin_view_on_gpu(dContext,
                                lazyView,
                                std::move(trampoline),
                                rrectToDraw,
                                dimensions,
                                xformedSigma)) {
            // In this case something has gone disastrously wrong so set up to drop the draw
            // that needed this resource and reduce future pollution of the cache.
            threadSafeCache->remove(key);
            return nullptr;
        }
    } else {
        view = threadSafeCache->find(key);
        if (view) {
            SkASSERT(view.asTextureProxy());
            SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
            return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
        }

        view = create_mask_on_cpu(rContext, rrectToDraw, dimensions, xformedSigma);
        if (!view) {
            return nullptr;
        }

        view = threadSafeCache->add(key, view);
    }

    SkASSERT(view.asTextureProxy());
    SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
    return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
}

static std::unique_ptr<GrFragmentProcessor> make_rrect_blur(GrRecordingContext* context,
                                                            float sigma,
                                                            float xformedSigma,
                                                            const SkRRect& srcRRect,
                                                            const SkRRect& devRRect) {
    SkASSERTF(!SkRRectPriv::IsCircle(devRRect),
              "Unexpected circle. %d\n\t%s\n\t%s",
              SkRRectPriv::IsCircle(srcRRect),
              srcRRect.dumpToString(true).c_str(),
              devRRect.dumpToString(true).c_str());
    SkASSERTF(!devRRect.isRect(),
              "Unexpected rect. %d\n\t%s\n\t%s",
              srcRRect.isRect(),
              srcRRect.dumpToString(true).c_str(),
              devRRect.dumpToString(true).c_str());

    // TODO: loosen this up
    if (!SkRRectPriv::IsSimpleCircular(devRRect)) {
        return nullptr;
    }

    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        return nullptr;
    }

    // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be sufficiently
    // small relative to both the size of the corner radius and the width (and height) of the rrect.
    SkRRect rrectToDraw;
    SkISize dimensions;
    SkScalar ignored[SkGpuBlurUtils::kBlurRRectMaxDivisions];

    bool ninePatchable = SkGpuBlurUtils::ComputeBlurredRRectParams(srcRRect,
                                                                   devRRect,
                                                                   sigma,
                                                                   xformedSigma,
                                                                   &rrectToDraw,
                                                                   &dimensions,
                                                                   ignored,
                                                                   ignored,
                                                                   ignored,
                                                                   ignored);
    if (!ninePatchable) {
        return nullptr;
    }

    std::unique_ptr<GrFragmentProcessor> maskFP =
            find_or_create_rrect_blur_mask_fp(context, rrectToDraw, dimensions, xformedSigma);
    if (!maskFP) {
        return nullptr;
    }

    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader ninePatchFP;"

        "uniform half cornerRadius;"
        "uniform float4 proxyRect;"
        "uniform half blurRadius;"

        "half4 main(float2 xy) {"
            // Warp the fragment position to the appropriate part of the 9-patch blur texture by
            // snipping out the middle section of the proxy rect.
            "float2 translatedFragPosFloat = sk_FragCoord.xy - proxyRect.LT;"
            "float2 proxyCenter = (proxyRect.RB - proxyRect.LT) * 0.5;"
            "half edgeSize = 2.0 * blurRadius + cornerRadius + 0.5;"

            // Position the fragment so that (0, 0) marks the center of the proxy rectangle.
            // Negative coordinates are on the left/top side and positive numbers are on the
            // right/bottom.
            "translatedFragPosFloat -= proxyCenter;"

            // Temporarily strip off the fragment's sign. x/y are now strictly increasing as we
            // move away from the center.
            "half2 fragDirection = half2(sign(translatedFragPosFloat));"
            "translatedFragPosFloat = abs(translatedFragPosFloat);"

            // Our goal is to snip out the "middle section" of the proxy rect (everything but the
            // edge). We've repositioned our fragment position so that (0, 0) is the centerpoint
            // and x/y are always positive, so we can subtract here and interpret negative results
            // as being within the middle section.
            "half2 translatedFragPosHalf = half2(translatedFragPosFloat - (proxyCenter - edgeSize));"

            // Remove the middle section by clamping to zero.
            "translatedFragPosHalf = max(translatedFragPosHalf, 0);"

            // Reapply the fragment's sign, so that negative coordinates once again mean left/top
            // side and positive means bottom/right side.
            "translatedFragPosHalf *= fragDirection;"

            // Offset the fragment so that (0, 0) marks the upper-left again, instead of the center
            // point.
            "translatedFragPosHalf += half2(edgeSize);"

            "half2 proxyDims = half2(2.0 * edgeSize);"
            "half2 texCoord = translatedFragPosHalf / proxyDims;"

            "return ninePatchFP.eval(texCoord).aaaa;"
        "}"
    );

    float cornerRadius = SkRRectPriv::GetSimpleRadii(devRRect).fX;
    float blurRadius = 3.f * SkScalarCeilToScalar(xformedSigma - 1 / 6.0f);
    SkRect proxyRect = devRRect.getBounds().makeOutset(blurRadius, blurRadius);

    auto rrectBlurFP = GrSkSLFP::Make(effect, "RRectBlur", /*inputFP=*/nullptr,
                                      GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                                      "ninePatchFP", GrSkSLFP::IgnoreOptFlags(std::move(maskFP)),
                                      "cornerRadius", cornerRadius,
                                      "proxyRect", proxyRect,
                                      "blurRadius", blurRadius);
    // Modulate blur with the input color.
    return GrBlendFragmentProcessor::Make<SkBlendMode::kModulate>(std::move(rrectBlurFP),
                                                                  /*dst=*/nullptr);
}

/**
 *  Try to directly render the mask filter into the target. Returns true if drawing was
 *  successful. If false is returned then paint is unmodified.
 */
static bool direct_filter_mask(GrRecordingContext* context,
                               const SkMaskFilterBase* maskFilter,
                               skgpu::ganesh::SurfaceDrawContext* sdc,
                               GrPaint&& paint,
                               const GrClip* clip,
                               const SkMatrix& viewMatrix,
                               const GrStyledShape& shape) {
    SkASSERT(sdc);
    if (maskFilter->type() != SkMaskFilterBase::Type::kBlur) {
        return false;
    }
    auto bmf = static_cast<const SkBlurMaskFilterImpl*>(maskFilter);

    if (bmf->blurStyle() != kNormal_SkBlurStyle) {
        return false;
    }

    // TODO: we could handle blurred stroked circles
    if (!shape.style().isSimpleFill()) {
        return false;
    }

    SkScalar xformedSigma = bmf->computeXformedSigma(viewMatrix);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        sdc->drawShape(clip, std::move(paint), GrAA::kYes, viewMatrix, GrStyledShape(shape));
        return true;
    }

    SkRRect srcRRect;
    bool inverted;
    if (!shape.asRRect(&srcRRect, nullptr, nullptr, &inverted) || inverted) {
        return false;
    }

    std::unique_ptr<GrFragmentProcessor> fp;

    SkRRect devRRect;
    bool devRRectIsValid = srcRRect.transform(viewMatrix, &devRRect);

    bool devRRectIsCircle = devRRectIsValid && SkRRectPriv::IsCircle(devRRect);

    bool canBeRect = srcRRect.isRect() && viewMatrix.preservesRightAngles();
    bool canBeCircle = (SkRRectPriv::IsCircle(srcRRect) && viewMatrix.isSimilarity()) ||
                       devRRectIsCircle;

    if (canBeRect || canBeCircle) {
        if (canBeRect) {
            fp = make_rect_blur(context, *context->priv().caps()->shaderCaps(),
                                srcRRect.rect(), viewMatrix, xformedSigma);
        } else {
            SkRect devBounds;
            if (devRRectIsCircle) {
                devBounds = devRRect.getBounds();
            } else {
                SkPoint center = {srcRRect.getBounds().centerX(), srcRRect.getBounds().centerY()};
                viewMatrix.mapPoints(&center, 1);
                SkScalar radius = viewMatrix.mapVector(0, srcRRect.width()/2.f).length();
                devBounds = {center.x() - radius,
                             center.y() - radius,
                             center.x() + radius,
                             center.y() + radius};
            }
            fp = make_circle_blur(context, devBounds, xformedSigma);
        }

        if (!fp) {
            return false;
        }

        SkRect srcProxyRect = srcRRect.rect();
        // Determine how much to outset the src rect to ensure we hit pixels within three sigma.
        SkScalar outsetX = 3.0f*xformedSigma;
        SkScalar outsetY = 3.0f*xformedSigma;
        if (viewMatrix.isScaleTranslate()) {
            outsetX /= SkScalarAbs(viewMatrix.getScaleX());
            outsetY /= SkScalarAbs(viewMatrix.getScaleY());
        } else {
            SkSize scale;
            if (!viewMatrix.decomposeScale(&scale, nullptr)) {
                return false;
            }
            outsetX /= scale.width();
            outsetY /= scale.height();
        }
        srcProxyRect.outset(outsetX, outsetY);

        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
        return true;
    }
    if (!viewMatrix.isScaleTranslate()) {
        return false;
    }
    if (!devRRectIsValid || !SkRRectPriv::AllCornersCircular(devRRect)) {
        return false;
    }

    fp = make_rrect_blur(context, bmf->sigma(), xformedSigma, srcRRect, devRRect);
    if (!fp) {
        return false;
    }

    if (!bmf->ignoreXform()) {
        SkRect srcProxyRect = srcRRect.rect();
        srcProxyRect.outset(3.0f*bmf->sigma(), 3.0f*bmf->sigma());
        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
    } else {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            return false;
        }

        SkIRect proxyBounds;
        float extra=3.f*SkScalarCeilToScalar(xformedSigma-1/6.0f);
        devRRect.rect().makeOutset(extra, extra).roundOut(&proxyBounds);

        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->fillPixelsWithLocalMatrix(clip, std::move(paint), proxyBounds, inverse);
    }

    return true;
}

// The key and clip-bounds are computed together because the caching decision can impact the
// clip-bound - since we only cache un-clipped masks the clip can be removed entirely.
// A 'false' return value indicates that the shape is known to be clipped away.
static bool compute_key_and_clip_bounds(skgpu::UniqueKey* maskKey,
                                        SkIRect* boundsForClip,
                                        const GrCaps* caps,
                                        const SkMatrix& viewMatrix,
                                        bool inverseFilled,
                                        const SkMaskFilterBase* maskFilter,
                                        const GrStyledShape& shape,
                                        const SkIRect& unclippedDevShapeBounds,
                                        const SkIRect& devClipBounds) {
    SkASSERT(maskFilter);
    *boundsForClip = devClipBounds;

#ifndef SK_DISABLE_MASKFILTERED_MASK_CACHING
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = !inverseFilled && viewMatrix.preservesAxisAlignment() &&
                    shape.hasUnstyledKey() && as_MFB(maskFilter)->asABlur(nullptr);

    if (useCache) {
        SkIRect clippedMaskRect, unClippedMaskRect;
        can_filter_mask(maskFilter, shape, unclippedDevShapeBounds, devClipBounds,
                        viewMatrix, &clippedMaskRect);
        if (clippedMaskRect.isEmpty()) {
            return false;
        }
        can_filter_mask(maskFilter, shape, unclippedDevShapeBounds, unclippedDevShapeBounds,
                        viewMatrix, &unClippedMaskRect);

        // Use the cache only if >50% of the filtered mask is visible.
        int unclippedWidth = unClippedMaskRect.width();
        int unclippedHeight = unClippedMaskRect.height();
        int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
        int64_t clippedArea = sk_64_mul(clippedMaskRect.width(), clippedMaskRect.height());
        int maxTextureSize = caps->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            // Make the clip not affect the mask
            *boundsForClip = unclippedDevShapeBounds;
        }
    }

    if (useCache) {
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(maskKey, kDomain, 5 + 2 + shape.unstyledKeySize(),
                                          "Mask Filtered Masks");

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = viewMatrix.get(SkMatrix::kMScaleX);
        SkScalar sy = viewMatrix.get(SkMatrix::kMScaleY);
        SkScalar kx = viewMatrix.get(SkMatrix::kMSkewX);
        SkScalar ky = viewMatrix.get(SkMatrix::kMSkewY);
        SkScalar tx = viewMatrix.get(SkMatrix::kMTransX);
        SkScalar ty = viewMatrix.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning. But, note that we're allowing
        // reuse for integer translations.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;

        builder[0] = SkFloat2Bits(sx);
        builder[1] = SkFloat2Bits(sy);
        builder[2] = SkFloat2Bits(kx);
        builder[3] = SkFloat2Bits(ky);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = shape.style().isSimpleHairline()
                                    ? ((shape.style().strokeRec().getCap() << 1) | 1)
                                    : 0;
        builder[4] = fracX | (fracY >> 8) | (styleBits << 16);

        SkMaskFilterBase::BlurRec rec;
        SkAssertResult(as_MFB(maskFilter)->asABlur(&rec));

        builder[5] = rec.fStyle;  // TODO: we could put this with the other style bits
        builder[6] = SkFloat2Bits(rec.fSigma);
        shape.writeUnstyledKey(&builder[7]);
    }
#endif

    return true;
}

/**
 * This function is used to implement filters that require an explicit src mask. It should only
 * be called if can_filter_mask returned true and the maskRect param should be the output from
 * that call.
 * Implementations are free to get the GrContext from the src texture in order to create
 * additional textures and perform multiple passes.
 */
static GrSurfaceProxyView filter_mask(GrRecordingContext* context,
                                      const SkMaskFilterBase* maskFilter,
                                      GrSurfaceProxyView srcView,
                                      GrColorType srcColorType,
                                      SkAlphaType srcAlphaType,
                                      const SkMatrix& ctm,
                                      const SkIRect& maskRect) {
    if (maskFilter->type() != SkMaskFilterBase::Type::kBlur) {
        return {};
    }
    auto bmf = static_cast<const SkBlurMaskFilterImpl*>(maskFilter);
    // 'maskRect' isn't snapped to the UL corner but the mask in 'src' is.
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());

    SkScalar xformedSigma = bmf->computeXformedSigma(ctm);

    // If we're doing a normal blur, we can clobber the pathTexture in the
    // gaussianBlur.  Otherwise, we need to save it for later compositing.
    bool isNormalBlur = (kNormal_SkBlurStyle == bmf->blurStyle());
    auto srcBounds = SkIRect::MakeSize(srcView.proxy()->dimensions());
    auto surfaceDrawContext = SkGpuBlurUtils::GaussianBlur(context,
                                                            srcView,
                                                            srcColorType,
                                                            srcAlphaType,
                                                            nullptr,
                                                            clipRect,
                                                            srcBounds,
                                                            xformedSigma,
                                                            xformedSigma,
                                                            SkTileMode::kClamp);
    if (!surfaceDrawContext || !surfaceDrawContext->asTextureProxy()) {
        return {};
    }

    if (!isNormalBlur) {
        GrPaint paint;
        // Blend pathTexture over blurTexture.
        paint.setCoverageFragmentProcessor(GrTextureEffect::Make(std::move(srcView), srcAlphaType));
        if (kInner_SkBlurStyle == bmf->blurStyle()) {
            // inner:  dst = dst * src
            paint.setCoverageSetOpXPFactory(SkRegion::kIntersect_Op);
        } else if (kSolid_SkBlurStyle == bmf->blurStyle()) {
            // solid:  dst = src + dst - src * dst
            //             = src + (1 - src) * dst
            paint.setCoverageSetOpXPFactory(SkRegion::kUnion_Op);
        } else if (kOuter_SkBlurStyle == bmf->blurStyle()) {
            // outer:  dst = dst * (1 - src)
            //             = 0 * src + (1 - src) * dst
            paint.setCoverageSetOpXPFactory(SkRegion::kDifference_Op);
        } else {
            paint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);
        }

        surfaceDrawContext->fillPixelsWithLocalMatrix(nullptr, std::move(paint), clipRect,
                                                      SkMatrix::I());
    }

    return surfaceDrawContext->readSurfaceView();
}

static GrSurfaceProxyView hw_create_filtered_mask(GrDirectContext* dContext,
                                                  skgpu::ganesh::SurfaceDrawContext* sdc,
                                                  const SkMatrix& viewMatrix,
                                                  const GrStyledShape& shape,
                                                  const SkMaskFilterBase* filter,
                                                  const SkIRect& unclippedDevShapeBounds,
                                                  const SkIRect& clipBounds,
                                                  SkIRect* maskRect,
                                                  skgpu::UniqueKey* key) {
    if (!can_filter_mask(filter, shape, unclippedDevShapeBounds, clipBounds, viewMatrix,
                         maskRect)) {
        return {};
    }

    if (clip_bounds_quick_reject(clipBounds, *maskRect)) {
        // clipped out
        return {};
    }

    auto threadSafeCache = dContext->priv().threadSafeCache();

    GrSurfaceProxyView lazyView;
    sk_sp<GrThreadSafeCache::Trampoline> trampoline;

    if (key->isValid()) {
        // In this case, we want GPU-filtered masks to have priority over SW-generated ones so
        // we pre-emptively add a lazy-view to the cache and fill it in later.
        std::tie(lazyView, trampoline) = GrThreadSafeCache::CreateLazyView(
                dContext, GrColorType::kAlpha_8, maskRect->size(),
                kMaskOrigin, SkBackingFit::kApprox);
        if (!lazyView) {
            return {}; // fall back to a SW-created mask - 'create_mask_GPU' probably won't succeed
        }

        key->setCustomData(create_data(*maskRect, unclippedDevShapeBounds));
        auto [cachedView, data] = threadSafeCache->findOrAddWithData(*key, lazyView);
        if (cachedView != lazyView) {
            // In this case, the gpu-thread lost out to a recording thread - use its result.
            SkASSERT(data);
            SkASSERT(cachedView.asTextureProxy());
            SkASSERT(cachedView.origin() == kMaskOrigin);

            *maskRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
            return cachedView;
        }
    }

    std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> maskSDC(
            create_mask_GPU(dContext, *maskRect, viewMatrix, shape, sdc->numSamples()));
    if (!maskSDC) {
        if (key->isValid()) {
            // It is very unlikely that 'create_mask_GPU' will fail after 'CreateLazyView'
            // succeeded but, if it does, remove the lazy-view from the cache and fallback to
            // a SW-created mask. Note that any recording threads that glommed onto the
            // lazy-view will have to, later, drop those draws.
            threadSafeCache->remove(*key);
        }
        return {};
    }

    auto filteredMaskView = filter_mask(dContext, filter,
                                                  maskSDC->readSurfaceView(),
                                                  maskSDC->colorInfo().colorType(),
                                                  maskSDC->colorInfo().alphaType(),
                                                  viewMatrix,
                                                  *maskRect);
    if (!filteredMaskView) {
        if (key->isValid()) {
            // Remove the lazy-view from the cache and fallback to a SW-created mask. Note that
            // any recording threads that glommed onto the lazy-view will have to, later, drop
            // those draws.
            threadSafeCache->remove(*key);
        }
        return {};
    }

    if (key->isValid()) {
        SkASSERT(filteredMaskView.dimensions() == lazyView.dimensions());
        SkASSERT(filteredMaskView.swizzle() == lazyView.swizzle());
        SkASSERT(filteredMaskView.origin() == lazyView.origin());

        trampoline->fProxy = filteredMaskView.asTextureProxyRef();
        return lazyView;
    }

    return filteredMaskView;
}

static void draw_shape_with_mask_filter(GrRecordingContext* rContext,
                                        skgpu::ganesh::SurfaceDrawContext* sdc,
                                        const GrClip* clip,
                                        GrPaint&& paint,
                                        const SkMatrix& viewMatrix,
                                        const SkMaskFilterBase* maskFilter,
                                        const GrStyledShape& origShape) {
    SkASSERT(maskFilter);

    const GrStyledShape* shape = &origShape;
    SkTLazy<GrStyledShape> tmpShape;

    if (origShape.style().applies()) {
        SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
        if (styleScale == 0) {
            return;
        }

        tmpShape.init(origShape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale));
        if (tmpShape->isEmpty()) {
            return;
        }

        shape = tmpShape.get();
    }

    if (direct_filter_mask(rContext, maskFilter, sdc, std::move(paint), clip, viewMatrix, *shape)) {
        // the mask filter was able to draw itself directly, so there's nothing
        // left to do.
        return;
    }
    assert_alive(paint);

    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = shape->inverseFilled() &&
                         !GrIsStrokeHairlineOrEquivalent(shape->style(), viewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, devClipBounds;
    if (!get_shape_and_clip_bounds(sdc, clip, *shape, viewMatrix,
                                   &unclippedDevShapeBounds, &devClipBounds)) {
        // TODO: just cons up an opaque mask here
        if (!inverseFilled) {
            return;
        }
    }

    skgpu::UniqueKey maskKey;
    SkIRect boundsForClip;
    if (!compute_key_and_clip_bounds(&maskKey, &boundsForClip,
                                     sdc->caps(),
                                     viewMatrix, inverseFilled,
                                     maskFilter, *shape,
                                     unclippedDevShapeBounds,
                                     devClipBounds)) {
        return; // 'shape' was entirely clipped out
    }

    GrSurfaceProxyView filteredMaskView;
    SkIRect maskRect;

    if (auto dContext = rContext->asDirectContext()) {
        filteredMaskView = hw_create_filtered_mask(dContext, sdc,
                                                   viewMatrix, *shape, maskFilter,
                                                   unclippedDevShapeBounds, boundsForClip,
                                                   &maskRect, &maskKey);
        if (filteredMaskView) {
            if (draw_mask(sdc, clip, viewMatrix, maskRect, std::move(paint),
                          std::move(filteredMaskView))) {
                // This path is completely drawn
                return;
            }
            assert_alive(paint);
        }
    }

    // Either HW mask rendering failed or we're in a DDL recording thread
    filteredMaskView = sw_create_filtered_mask(rContext,
                                               viewMatrix, *shape, maskFilter,
                                               unclippedDevShapeBounds, boundsForClip,
                                               &maskRect, &maskKey);
    if (filteredMaskView) {
        if (draw_mask(sdc, clip, viewMatrix, maskRect, std::move(paint),
                      std::move(filteredMaskView))) {
            return;
        }
        assert_alive(paint);
    }
}

void GrBlurUtils::drawShapeWithMaskFilter(GrRecordingContext* rContext,
                                          skgpu::ganesh::SurfaceDrawContext* sdc,
                                          const GrClip* clip,
                                          const GrStyledShape& shape,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkMaskFilter* mf) {
    draw_shape_with_mask_filter(rContext, sdc, clip, std::move(paint),
                                viewMatrix, as_MFB(mf), shape);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrRecordingContext* rContext,
                                          skgpu::ganesh::SurfaceDrawContext* sdc,
                                          const GrClip* clip,
                                          const SkPaint& paint,
                                          const SkMatrixProvider& matrixProvider,
                                          const GrStyledShape& shape) {
    if (rContext->abandoned()) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(rContext,
                          sdc->colorInfo(),
                          paint,
                          matrixProvider.localToDevice(),
                          sdc->surfaceProps(),
                          &grPaint)) {
        return;
    }

    const SkMatrix& viewMatrix(matrixProvider.localToDevice());
    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf && !GrFragmentProcessors::IsSupported(mf)) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_shape_with_mask_filter(rContext, sdc, clip, std::move(grPaint), viewMatrix, mf, shape);
    } else {
        sdc->drawShape(clip, std::move(grPaint), sdc->chooseAA(paint), viewMatrix,
                       GrStyledShape(shape));
    }
}
