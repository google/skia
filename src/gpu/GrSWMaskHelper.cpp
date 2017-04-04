/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTargetContext.h"
#include "GrShape.h"
#include "GrSurfaceContext.h"
#include "GrTextureProxy.h"
#include "ops/GrDrawOp.h"

#include "SkDistanceFieldGen.h"

#include "ops/GrRectOpFactory.h"

/*
 * Convert a boolean operation into a transfer mode code
 */
static SkBlendMode op_to_mode(SkRegion::Op op) {

    static const SkBlendMode modeMap[] = {
        SkBlendMode::kDstOut,   // kDifference_Op
        SkBlendMode::kModulate, // kIntersect_Op
        SkBlendMode::kSrcOver,  // kUnion_Op
        SkBlendMode::kXor,      // kXOR_Op
        SkBlendMode::kClear,    // kReverseDifference_Op
        SkBlendMode::kSrc,      // kReplace_Op
    };

    return modeMap[op];
}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawRect(const SkRect& rect, SkRegion::Op op, GrAA aa, uint8_t alpha) {
    SkPaint paint;

    paint.setBlendMode(op_to_mode(op));
    paint.setAntiAlias(GrAA::kYes == aa);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawRect(rect, paint);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawShape(const GrShape& shape, SkRegion::Op op, GrAA aa, uint8_t alpha) {
    SkPaint paint;
    paint.setPathEffect(shape.style().refPathEffect());
    shape.style().strokeRec().applyToPaint(&paint);
    paint.setAntiAlias(GrAA::kYes == aa);

    SkPath path;
    shape.asPath(&path);
    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        paint.setBlendMode(op_to_mode(op));
        paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
        fDraw.drawPath(path, paint);
    }
}

bool GrSWMaskHelper::init(const SkIRect& resultBounds, const SkMatrix* matrix) {
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }

    // Now translate so the bound's UL corner is at the origin
    fMatrix.postTranslate(-SkIntToScalar(resultBounds.fLeft), -SkIntToScalar(resultBounds.fTop));
    SkIRect bounds = SkIRect::MakeWH(resultBounds.width(), resultBounds.height());

    const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(bounds.width(), bounds.height());
    if (!fPixels.tryAlloc(bmImageInfo)) {
        return false;
    }
    fPixels.erase(0);

    sk_bzero(&fDraw, sizeof(fDraw));
    fDraw.fDst      = fPixels;
    fRasterClip.setRect(bounds);
    fDraw.fRC       = &fRasterClip;
    fDraw.fMatrix   = &fMatrix;
    return true;
}

sk_sp<GrTextureProxy> GrSWMaskHelper::toTextureProxy(GrContext* context, SkBackingFit fit) {
    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = fPixels.width();
    desc.fHeight = fPixels.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeDeferredSurfaceContext(
                                                                                desc,
                                                                                fit,
                                                                                SkBudgeted::kYes);
    if (!sContext || !sContext->asTextureProxy()) {
        return nullptr;
    }

    SkImageInfo ii = SkImageInfo::MakeA8(desc.fWidth, desc.fHeight);
    if (!sContext->writePixels(ii, fPixels.addr(), fPixels.rowBytes(), 0, 0)) {
        return nullptr;
    }

    return sContext->asTextureProxyRef();
}

/**
 * Convert mask generation results to a signed distance field
 */
void GrSWMaskHelper::toSDF(unsigned char* sdf) {
    SkGenerateDistanceFieldFromA8Image(sdf, (const unsigned char*)fPixels.addr(),
                                       fPixels.width(), fPixels.height(), fPixels.rowBytes());
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Software rasterizes shape to A8 mask and uploads the result to a scratch texture. Returns the
 * resulting texture on success; nullptr on failure.
 */
sk_sp<GrTextureProxy> GrSWMaskHelper::DrawShapeMaskToTexture(GrContext* context,
                                                             const GrShape& shape,
                                                             const SkIRect& resultBounds,
                                                             GrAA aa,
                                                             SkBackingFit fit,
                                                             const SkMatrix* matrix) {
    GrSWMaskHelper helper;

    if (!helper.init(resultBounds, matrix)) {
        return nullptr;
    }

    helper.drawShape(shape, SkRegion::kReplace_Op, aa, 0xFF);

    return helper.toTextureProxy(context, fit);
}

void GrSWMaskHelper::DrawToTargetWithShapeMask(sk_sp<GrTextureProxy> proxy,
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

    GrResourceProvider* resourceProvider = renderTargetContext->resourceProvider();

    SkRect dstRect = SkRect::Make(deviceSpaceRectToDraw);

    // We use device coords to compute the texture coordinates. We take the device coords and apply
    // a translation so that the top-left of the device bounds maps to 0,0, and then a scaling
    // matrix to normalized coords.
    SkMatrix maskMatrix = SkMatrix::MakeTrans(SkIntToScalar(-textureOriginInDeviceSpace.fX),
                                              SkIntToScalar(-textureOriginInDeviceSpace.fY));
    maskMatrix.preConcat(viewMatrix);
    std::unique_ptr<GrLegacyMeshDrawOp> op = GrRectOpFactory::MakeNonAAFill(
            paint.getColor(), SkMatrix::I(), dstRect, nullptr, &invert);
    paint.addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(
            resourceProvider, std::move(proxy), nullptr, maskMatrix,
            GrSamplerParams::kNone_FilterMode));
    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    pipelineBuilder.setUserStencil(&userStencilSettings);
    renderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
}
