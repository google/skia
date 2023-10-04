/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/SoftwarePathRenderer.h"

#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkSemaphore.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrDeferredProxyUploader.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSWMaskHelper.h"
#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"

namespace {

/**
 * Payload class for use with GrTDeferredProxyUploader. The software path renderer only draws
 * a single path into the mask texture. This stores all of the information needed by the worker
 * thread's call to drawShape (see below, in onDrawPath).
 */
class SoftwarePathData {
public:
    SoftwarePathData(const SkIRect& maskBounds, const SkMatrix& viewMatrix,
                     const GrStyledShape& shape, GrAA aa)
            : fMaskBounds(maskBounds)
            , fViewMatrix(viewMatrix)
            , fShape(shape)
            , fAA(aa) {}

    const SkIRect& getMaskBounds() const { return fMaskBounds; }
    const SkMatrix* getViewMatrix() const { return &fViewMatrix; }
    const GrStyledShape& getShape() const { return fShape; }
    GrAA getAA() const { return fAA; }

private:
    SkIRect fMaskBounds;
    SkMatrix fViewMatrix;
    GrStyledShape fShape;
    GrAA fAA;
};

bool get_unclipped_shape_dev_bounds(const GrStyledShape& shape, const SkMatrix& matrix,
                                    SkIRect* devBounds) {
    SkRect shapeBounds = shape.styledBounds();
    if (shapeBounds.isEmpty()) {
        return false;
    }
    SkRect shapeDevBounds;
    matrix.mapRect(&shapeDevBounds, shapeBounds);
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

GrSurfaceProxyView make_deferred_mask_texture_view(GrRecordingContext* rContext,
                                                   SkBackingFit fit,
                                                   SkISize dimensions) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    const GrCaps* caps = rContext->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                                 GrRenderable::kNo);

    skgpu::Swizzle swizzle = caps->getReadSwizzle(format, GrColorType::kAlpha_8);

    auto proxy = proxyProvider->createProxy(format,
                                            dimensions,
                                            GrRenderable::kNo,
                                            1,
                                            skgpu::Mipmapped::kNo,
                                            fit,
                                            skgpu::Budgeted::kYes,
                                            GrProtected::kNo,
                                            /*label=*/"MakeDeferredMaskTextureView");
    return {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
}


} // anonymous namespace

namespace skgpu::ganesh {

////////////////////////////////////////////////////////////////////////////////
PathRenderer::CanDrawPath SoftwarePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // Pass on any style that applies. The caller will apply the style if a suitable renderer is
    // not found and try again with the new GrStyledShape.
    if (!args.fShape->style().applies() && SkToBool(fProxyProvider) &&
        (args.fAAType == GrAAType::kCoverage || args.fAAType == GrAAType::kNone)) {
        // This is the fallback renderer for when a path is too complicated for the GPU ones.
        return CanDrawPath::kAsBackup;
    }
    return CanDrawPath::kNo;
}

////////////////////////////////////////////////////////////////////////////////

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
bool SoftwarePathRenderer::GetShapeAndClipBounds(SurfaceDrawContext* sdc,
                                                 const GrClip* clip,
                                                 const GrStyledShape& shape,
                                                 const SkMatrix& matrix,
                                                 SkIRect* unclippedDevShapeBounds,
                                                 SkIRect* clippedDevShapeBounds,
                                                 SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    *devClipBounds = clip ? clip->getConservativeBounds()
                          : SkIRect::MakeWH(sdc->width(), sdc->height());

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::MakeEmpty();
        *clippedDevShapeBounds = SkIRect::MakeEmpty();
        return false;
    }
    if (!clippedDevShapeBounds->intersect(*devClipBounds, *unclippedDevShapeBounds)) {
        *clippedDevShapeBounds = SkIRect::MakeEmpty();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void SoftwarePathRenderer::DrawNonAARect(SurfaceDrawContext* sdc,
                                         GrPaint&& paint,
                                         const GrUserStencilSettings& userStencilSettings,
                                         const GrClip* clip,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkMatrix& localMatrix) {
    sdc->stencilRect(clip, &userStencilSettings, std::move(paint), GrAA::kNo,
                     viewMatrix, rect, &localMatrix);
}

void SoftwarePathRenderer::DrawAroundInvPath(SurfaceDrawContext* sdc,
                                             GrPaint&& paint,
                                             const GrUserStencilSettings& userStencilSettings,
                                             const GrClip* clip,
                                             const SkMatrix& viewMatrix,
                                             const SkIRect& devClipBounds,
                                             const SkIRect& devPathBounds) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    SkRect rect;
    if (devClipBounds.fTop < devPathBounds.fTop) {
        rect.setLTRB(SkIntToScalar(devClipBounds.fLeft),  SkIntToScalar(devClipBounds.fTop),
                     SkIntToScalar(devClipBounds.fRight), SkIntToScalar(devPathBounds.fTop));
        DrawNonAARect(sdc, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.setLTRB(SkIntToScalar(devClipBounds.fLeft), SkIntToScalar(devPathBounds.fTop),
                     SkIntToScalar(devPathBounds.fLeft), SkIntToScalar(devPathBounds.fBottom));
        DrawNonAARect(sdc, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.setLTRB(SkIntToScalar(devPathBounds.fRight), SkIntToScalar(devPathBounds.fTop),
                     SkIntToScalar(devClipBounds.fRight), SkIntToScalar(devPathBounds.fBottom));
        DrawNonAARect(sdc, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.setLTRB(SkIntToScalar(devClipBounds.fLeft),  SkIntToScalar(devPathBounds.fBottom),
                     SkIntToScalar(devClipBounds.fRight), SkIntToScalar(devClipBounds.fBottom));
        DrawNonAARect(sdc, std::move(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
}

void SoftwarePathRenderer::DrawToTargetWithShapeMask(
        GrSurfaceProxyView view,
        SurfaceDrawContext* sdc,
        GrPaint&& paint,
        const GrUserStencilSettings& userStencilSettings,
        const GrClip* clip,
        const SkMatrix& viewMatrix,
        const SkIPoint& textureOriginInDeviceSpace,
        const SkIRect& deviceSpaceRectToDraw) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    view.concatSwizzle(skgpu::Swizzle("aaaa"));

    SkRect dstRect = SkRect::Make(deviceSpaceRectToDraw);

    // We use device coords to compute the texture coordinates. We take the device coords and apply
    // a translation so that the top-left of the device bounds maps to 0,0, and then a scaling
    // matrix to normalized coords.
    SkMatrix maskMatrix = SkMatrix::Translate(SkIntToScalar(-textureOriginInDeviceSpace.fX),
                                              SkIntToScalar(-textureOriginInDeviceSpace.fY));
    maskMatrix.preConcat(viewMatrix);

    paint.setCoverageFragmentProcessor(GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, maskMatrix, GrSamplerState::Filter::kNearest));
    DrawNonAARect(sdc, std::move(paint), userStencilSettings, clip, SkMatrix::I(),
                  dstRect, invert);
}

////////////////////////////////////////////////////////////////////////////////
// return true on success; false on failure
bool SoftwarePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "SoftwarePathRenderer::onDrawPath");

    if (!fProxyProvider) {
        return false;
    }

    SkASSERT(!args.fShape->style().applies());
    // We really need to know if the shape will be inverse filled or not
    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = args.fShape->inverseFilled() &&
                        !GrIsStrokeHairlineOrEquivalent(args.fShape->style(),
                                                        *args.fViewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, clippedDevShapeBounds, devClipBounds;
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = fAllowCaching && !inverseFilled && args.fViewMatrix->preservesAxisAlignment() &&
                    args.fShape->hasUnstyledKey() && (GrAAType::kCoverage == args.fAAType);

    if (!GetShapeAndClipBounds(args.fSurfaceDrawContext,
                               args.fClip, *args.fShape,
                               *args.fViewMatrix, &unclippedDevShapeBounds,
                               &clippedDevShapeBounds,
                               &devClipBounds)) {
        if (inverseFilled) {
            DrawAroundInvPath(args.fSurfaceDrawContext, std::move(args.fPaint),
                              *args.fUserStencilSettings, args.fClip, *args.fViewMatrix,
                              devClipBounds, unclippedDevShapeBounds);
        }
        return true;
    }

    const SkIRect* boundsForMask = &clippedDevShapeBounds;
    if (useCache) {
        // Use the cache only if >50% of the path is visible.
        int unclippedWidth = unclippedDevShapeBounds.width();
        int unclippedHeight = unclippedDevShapeBounds.height();
        int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
        int64_t clippedArea = sk_64_mul(clippedDevShapeBounds.width(),
                                        clippedDevShapeBounds.height());
        int maxTextureSize = args.fSurfaceDrawContext->caps()->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            boundsForMask = &unclippedDevShapeBounds;
        }
    }

    skgpu::UniqueKey maskKey;
    if (useCache) {
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = args.fViewMatrix->get(SkMatrix::kMScaleX);
        SkScalar sy = args.fViewMatrix->get(SkMatrix::kMScaleY);
        SkScalar kx = args.fViewMatrix->get(SkMatrix::kMSkewX);
        SkScalar ky = args.fViewMatrix->get(SkMatrix::kMSkewY);
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 7 + args.fShape->unstyledKeySize(),
                                     "SW Path Mask");
        builder[0] = boundsForMask->width();
        builder[1] = boundsForMask->height();

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // Fractional translate does not affect caching on Android. This is done for better cache
        // hit ratio and speed, but it is matching HWUI behavior, which doesn't consider the matrix
        // at all when caching paths.
        SkFixed fracX = 0;
        SkFixed fracY = 0;
#else
        SkScalar tx = args.fViewMatrix->get(SkMatrix::kMTransX);
        SkScalar ty = args.fViewMatrix->get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
#endif
        builder[2] = SkFloat2Bits(sx);
        builder[3] = SkFloat2Bits(sy);
        builder[4] = SkFloat2Bits(kx);
        builder[5] = SkFloat2Bits(ky);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = args.fShape->style().isSimpleHairline() ?
                             ((args.fShape->style().strokeRec().getCap() << 1) | 1) : 0;
        builder[6] = fracX | (fracY >> 8) | (styleBits << 16);
        args.fShape->writeUnstyledKey(&builder[7]);
    }

    GrSurfaceProxyView view;
    if (useCache) {
        sk_sp<GrTextureProxy> proxy = fProxyProvider->findOrCreateProxyByUniqueKey(maskKey);
        if (proxy) {
            skgpu::Swizzle swizzle = args.fSurfaceDrawContext->caps()->getReadSwizzle(
                    proxy->backendFormat(), GrColorType::kAlpha_8);
            view = {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
            args.fContext->priv().stats()->incNumPathMasksCacheHits();
        }
    }
    if (!view) {
        SkBackingFit fit = useCache ? SkBackingFit::kExact : SkBackingFit::kApprox;
        GrAA aa = GrAA(GrAAType::kCoverage == args.fAAType);

        SkTaskGroup* taskGroup = nullptr;
        if (auto direct = args.fContext->asDirectContext()) {
            taskGroup = direct->priv().getTaskGroup();
        }

        if (taskGroup) {
            view = make_deferred_mask_texture_view(args.fContext, fit, boundsForMask->size());
            if (!view) {
                return false;
            }

            auto uploader = std::make_unique<GrTDeferredProxyUploader<SoftwarePathData>>(
                    *boundsForMask, *args.fViewMatrix, *args.fShape, aa);
            GrTDeferredProxyUploader<SoftwarePathData>* uploaderRaw = uploader.get();

            auto drawAndUploadMask = [uploaderRaw] {
                TRACE_EVENT0("skia.gpu", "Threaded SW Mask Render");
                GrSWMaskHelper helper(uploaderRaw->getPixels());
                if (helper.init(uploaderRaw->data().getMaskBounds())) {
                    helper.drawShape(uploaderRaw->data().getShape(),
                                     *uploaderRaw->data().getViewMatrix(),
                                     uploaderRaw->data().getAA(), 0xFF);
                } else {
                    SkDEBUGFAIL("Unable to allocate SW mask.");
                }
                uploaderRaw->signalAndFreeData();
            };
            taskGroup->add(std::move(drawAndUploadMask));
            view.asTextureProxy()->texPriv().setDeferredUploader(std::move(uploader));
        } else {
            GrSWMaskHelper helper;
            if (!helper.init(*boundsForMask)) {
                return false;
            }
            helper.drawShape(*args.fShape, *args.fViewMatrix, aa, 0xFF);
            view = helper.toTextureView(args.fContext, fit);
        }

        if (!view) {
            return false;
        }
        if (useCache) {
            SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);

            // We will add an invalidator to the path so that if the path goes away we will
            // delete or recycle the mask texture.
            auto listener = GrMakeUniqueKeyInvalidationListener(&maskKey,
                                                                args.fContext->priv().contextID());
            fProxyProvider->assignUniqueKeyToProxy(maskKey, view.asTextureProxy());
            args.fShape->addGenIDChangeListener(std::move(listener));
        }

        args.fContext->priv().stats()->incNumPathMasksGenerated();
    }
    SkASSERT(view);
    if (inverseFilled) {
        DrawAroundInvPath(args.fSurfaceDrawContext, GrPaint::Clone(args.fPaint),
                          *args.fUserStencilSettings, args.fClip, *args.fViewMatrix, devClipBounds,
                          unclippedDevShapeBounds);
    }
    DrawToTargetWithShapeMask(std::move(view), args.fSurfaceDrawContext, std::move(args.fPaint),
                              *args.fUserStencilSettings, args.fClip, *args.fViewMatrix,
                              SkIPoint{boundsForMask->fLeft, boundsForMask->fTop}, *boundsForMask);

    return true;
}

}  // namespace skgpu::ganesh
