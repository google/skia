/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSemaphore.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrSWMaskHelper.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrDrawOp.h"

////////////////////////////////////////////////////////////////////////////////
GrPathRenderer::CanDrawPath
GrSoftwarePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // Pass on any style that applies. The caller will apply the style if a suitable renderer is
    // not found and try again with the new GrShape.
    if (!args.fShape->style().applies() && SkToBool(fProxyProvider) &&
        (args.fAAType == GrAAType::kCoverage || args.fAAType == GrAAType::kNone)) {
        // This is the fallback renderer for when a path is too complicated for the GPU ones.
        return CanDrawPath::kAsBackup;
    }
    return CanDrawPath::kNo;
}

////////////////////////////////////////////////////////////////////////////////
static bool get_unclipped_shape_dev_bounds(const GrShape& shape, const SkMatrix& matrix,
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

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
bool GrSoftwarePathRenderer::GetShapeAndClipBounds(GrRenderTargetContext* renderTargetContext,
                                                   const GrClip& clip,
                                                   const GrShape& shape,
                                                   const SkMatrix& matrix,
                                                   SkIRect* unclippedDevShapeBounds,
                                                   SkIRect* clippedDevShapeBounds,
                                                   SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    clip.getConservativeBounds(renderTargetContext->width(),
                               renderTargetContext->height(),
                               devClipBounds);

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::EmptyIRect();
        *clippedDevShapeBounds = SkIRect::EmptyIRect();
        return false;
    }
    if (!clippedDevShapeBounds->intersect(*devClipBounds, *unclippedDevShapeBounds)) {
        *clippedDevShapeBounds = SkIRect::EmptyIRect();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void GrSoftwarePathRenderer::DrawNonAARect(GrRenderTargetContext* renderTargetContext,
                                           GrPaint&& paint,
                                           const GrUserStencilSettings& userStencilSettings,
                                           const GrClip& clip,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const SkMatrix& localMatrix) {
    renderTargetContext->priv().stencilRect(clip, &userStencilSettings, std::move(paint), GrAA::kNo,
                                            viewMatrix, rect, &localMatrix);
}

void GrSoftwarePathRenderer::DrawAroundInvPath(GrRenderTargetContext* renderTargetContext,
                                               GrPaint&& paint,
                                               const GrUserStencilSettings& userStencilSettings,
                                               const GrClip& clip,
                                               const SkMatrix& viewMatrix,
                                               const SkIRect& devClipBounds,
                                               const SkIRect& devPathBounds) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    SkRect rect;
    if (devClipBounds.fTop < devPathBounds.fTop) {
        rect.iset(devClipBounds.fLeft, devClipBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fTop);
        DrawNonAARect(renderTargetContext, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fTop,
                  devPathBounds.fLeft, devPathBounds.fBottom);
        DrawNonAARect(renderTargetContext, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.iset(devPathBounds.fRight, devPathBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fBottom);
        DrawNonAARect(renderTargetContext, GrPaint::Clone(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fBottom,
                  devClipBounds.fRight, devClipBounds.fBottom);
        DrawNonAARect(renderTargetContext, std::move(paint), userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
}

void GrSoftwarePathRenderer::DrawToTargetWithShapeMask(
        sk_sp<GrTextureProxy> proxy,
        GrRenderTargetContext* renderTargetContext,
        GrPaint&& paint,
        const GrUserStencilSettings& userStencilSettings,
        const GrClip& clip,
        const SkMatrix& viewMatrix,
        const SkIPoint& textureOriginInDeviceSpace,
        const SkIRect& deviceSpaceRectToDraw) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    SkRect dstRect = SkRect::Make(deviceSpaceRectToDraw);

    // We use device coords to compute the texture coordinates. We take the device coords and apply
    // a translation so that the top-left of the device bounds maps to 0,0, and then a scaling
    // matrix to normalized coords.
    SkMatrix maskMatrix = SkMatrix::MakeTrans(SkIntToScalar(-textureOriginInDeviceSpace.fX),
                                              SkIntToScalar(-textureOriginInDeviceSpace.fY));
    maskMatrix.preConcat(viewMatrix);
    paint.addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(
            std::move(proxy), maskMatrix, GrSamplerState::Filter::kNearest));
    DrawNonAARect(renderTargetContext, std::move(paint), userStencilSettings, clip, SkMatrix::I(),
                  dstRect, invert);
}

static sk_sp<GrTextureProxy> make_deferred_mask_texture_proxy(GrRecordingContext* context,
                                                              SkBackingFit fit,
                                                              int width, int height) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = kAlpha_8_GrPixelConfig;

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                                 GrRenderable::kNo);

    return proxyProvider->createProxy(format, desc, GrRenderable::kNo, 1, kTopLeft_GrSurfaceOrigin,
                                      fit, SkBudgeted::kYes, GrProtected::kNo);
}

namespace {

/**
 * Payload class for use with GrTDeferredProxyUploader. The software path renderer only draws
 * a single path into the mask texture. This stores all of the information needed by the worker
 * thread's call to drawShape (see below, in onDrawPath).
 */
class SoftwarePathData {
public:
    SoftwarePathData(const SkIRect& maskBounds, const SkMatrix& viewMatrix, const GrShape& shape,
                     GrAA aa)
            : fMaskBounds(maskBounds)
            , fViewMatrix(viewMatrix)
            , fShape(shape)
            , fAA(aa) {}

    const SkIRect& getMaskBounds() const { return fMaskBounds; }
    const SkMatrix* getViewMatrix() const { return &fViewMatrix; }
    const GrShape& getShape() const { return fShape; }
    GrAA getAA() const { return fAA; }

private:
    SkIRect fMaskBounds;
    SkMatrix fViewMatrix;
    GrShape fShape;
    GrAA fAA;
};

// When the SkPathRef genID changes, invalidate a corresponding GrResource described by key.
class PathInvalidator : public SkPathRef::GenIDChangeListener {
public:
    PathInvalidator(const GrUniqueKey& key, uint32_t contextUniqueID)
            : fMsg(key, contextUniqueID) {}

private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void onChange() override {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg);
    }
};

}

////////////////////////////////////////////////////////////////////////////////
// return true on success; false on failure
bool GrSoftwarePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrSoftwarePathRenderer::onDrawPath");
    if (!fProxyProvider) {
        return false;
    }

    SkASSERT(!args.fShape->style().applies());
    // We really need to know if the shape will be inverse filled or not
    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = args.fShape->inverseFilled() &&
                        !IsStrokeHairlineOrEquivalent(args.fShape->style(),
                                                      *args.fViewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, clippedDevShapeBounds, devClipBounds;
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = fAllowCaching && !inverseFilled && args.fViewMatrix->preservesAxisAlignment() &&
                    args.fShape->hasUnstyledKey() && (GrAAType::kCoverage == args.fAAType);

    if (!GetShapeAndClipBounds(args.fRenderTargetContext,
                               *args.fClip, *args.fShape,
                               *args.fViewMatrix, &unclippedDevShapeBounds,
                               &clippedDevShapeBounds,
                               &devClipBounds)) {
        if (inverseFilled) {
            DrawAroundInvPath(args.fRenderTargetContext, std::move(args.fPaint),
                              *args.fUserStencilSettings, *args.fClip, *args.fViewMatrix,
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
        int maxTextureSize = args.fRenderTargetContext->caps()->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            boundsForMask = &unclippedDevShapeBounds;
        }
    }

    GrUniqueKey maskKey;
    if (useCache) {
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = args.fViewMatrix->get(SkMatrix::kMScaleX);
        SkScalar sy = args.fViewMatrix->get(SkMatrix::kMScaleY);
        SkScalar kx = args.fViewMatrix->get(SkMatrix::kMSkewX);
        SkScalar ky = args.fViewMatrix->get(SkMatrix::kMSkewY);
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&maskKey, kDomain, 5 + args.fShape->unstyledKeySize(),
                                     "SW Path Mask");
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
        builder[0] = SkFloat2Bits(sx);
        builder[1] = SkFloat2Bits(sy);
        builder[2] = SkFloat2Bits(kx);
        builder[3] = SkFloat2Bits(ky);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = args.fShape->style().isSimpleHairline() ?
                             ((args.fShape->style().strokeRec().getCap() << 1) | 1) : 0;
        builder[4] = fracX | (fracY >> 8) | (styleBits << 16);
        args.fShape->writeUnstyledKey(&builder[5]);
    }

    sk_sp<GrTextureProxy> proxy;
    if (useCache) {
        proxy = fProxyProvider->findOrCreateProxyByUniqueKey(maskKey, GrColorType::kAlpha_8,
                                                             kTopLeft_GrSurfaceOrigin);
    }
    if (!proxy) {
        SkBackingFit fit = useCache ? SkBackingFit::kExact : SkBackingFit::kApprox;
        GrAA aa = GrAA(GrAAType::kCoverage == args.fAAType);

        SkTaskGroup* taskGroup = nullptr;
        if (auto direct = args.fContext->priv().asDirectContext()) {
            taskGroup = direct->priv().getTaskGroup();
        }

        if (taskGroup) {
            proxy = make_deferred_mask_texture_proxy(args.fContext, fit,
                                                     boundsForMask->width(),
                                                     boundsForMask->height());
            if (!proxy) {
                return false;
            }

            auto uploader = skstd::make_unique<GrTDeferredProxyUploader<SoftwarePathData>>(
                    *boundsForMask, *args.fViewMatrix, *args.fShape, aa);
            GrTDeferredProxyUploader<SoftwarePathData>* uploaderRaw = uploader.get();

            auto drawAndUploadMask = [uploaderRaw] {
                TRACE_EVENT0("skia.gpu", "Threaded SW Mask Render");
                GrSWMaskHelper helper(uploaderRaw->getPixels());
                if (helper.init(uploaderRaw->data().getMaskBounds())) {
                    helper.drawShape(uploaderRaw->data().getShape(),
                                     *uploaderRaw->data().getViewMatrix(),
                                     SkRegion::kReplace_Op, uploaderRaw->data().getAA(), 0xFF);
                } else {
                    SkDEBUGFAIL("Unable to allocate SW mask.");
                }
                uploaderRaw->signalAndFreeData();
            };
            taskGroup->add(std::move(drawAndUploadMask));
            proxy->texPriv().setDeferredUploader(std::move(uploader));
        } else {
            GrSWMaskHelper helper;
            if (!helper.init(*boundsForMask)) {
                return false;
            }
            helper.drawShape(*args.fShape, *args.fViewMatrix, SkRegion::kReplace_Op, aa, 0xFF);
            proxy = helper.toTextureProxy(args.fContext, fit);
        }

        if (!proxy) {
            return false;
        }
        if (useCache) {
            SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
            fProxyProvider->assignUniqueKeyToProxy(maskKey, proxy.get());
            args.fShape->addGenIDChangeListener(
                    sk_make_sp<PathInvalidator>(maskKey, args.fContext->priv().contextID()));
        }
    }
    if (inverseFilled) {
        DrawAroundInvPath(args.fRenderTargetContext, GrPaint::Clone(args.fPaint),
                          *args.fUserStencilSettings, *args.fClip, *args.fViewMatrix, devClipBounds,
                          unclippedDevShapeBounds);
    }
    DrawToTargetWithShapeMask(
            std::move(proxy), args.fRenderTargetContext, std::move(args.fPaint),
            *args.fUserStencilSettings, *args.fClip, *args.fViewMatrix,
            SkIPoint{boundsForMask->fLeft, boundsForMask->fTop}, *boundsForMask);

    return true;
}
