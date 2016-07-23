/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "batches/GrDrawBatch.h"
#include "GrDrawContext.h"
#include "GrPipelineBuilder.h"
#include "GrShape.h"

#include "SkDistanceFieldGen.h"

#include "batches/GrRectBatchFactory.h"

/*
 * Convert a boolean operation into a transfer mode code
 */
static SkXfermode::Mode op_to_mode(SkRegion::Op op) {

    static const SkXfermode::Mode modeMap[] = {
        SkXfermode::kDstOut_Mode,   // kDifference_Op
        SkXfermode::kModulate_Mode, // kIntersect_Op
        SkXfermode::kSrcOver_Mode,  // kUnion_Op
        SkXfermode::kXor_Mode,      // kXOR_Op
        SkXfermode::kClear_Mode,    // kReverseDifference_Op
        SkXfermode::kSrc_Mode,      // kReplace_Op
    };

    return modeMap[op];
}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawRect(const SkRect& rect, SkRegion::Op op,
                              bool antiAlias, uint8_t alpha) {
    SkPaint paint;

    paint.setXfermode(SkXfermode::Make(op_to_mode(op)));
    paint.setAntiAlias(antiAlias);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawRect(rect, paint);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::drawShape(const GrShape& shape, SkRegion::Op op, bool antiAlias,
                               uint8_t alpha) {
    SkPaint paint;
    paint.setPathEffect(sk_ref_sp(shape.style().pathEffect()));
    shape.style().strokeRec().applyToPaint(&paint);
    paint.setAntiAlias(antiAlias);

    SkPath path;
    shape.asPath(&path);
    if (SkRegion::kReplace_Op == op && 0xFF == alpha) {
        SkASSERT(0xFF == paint.getAlpha());
        fDraw.drawPathCoverage(path, paint);
    } else {
        paint.setXfermodeMode(op_to_mode(op));
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

/**
 * Get a texture (from the texture cache) of the correct size & format.
 */
GrTexture* GrSWMaskHelper::createTexture() {
    GrSurfaceDesc desc;
    desc.fWidth = fPixels.width();
    desc.fHeight = fPixels.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    return fTexProvider->createApproxTexture(desc);
}

/**
 * Move the result of the software mask generation back to the gpu
 */
void GrSWMaskHelper::toTexture(GrTexture *texture) {
    // Since we're uploading to it, and it's compressed, 'texture' shouldn't
    // have a render target.
    SkASSERT(!texture->asRenderTarget());

    texture->writePixels(0, 0, fPixels.width(), fPixels.height(), texture->config(),
                         fPixels.addr(), fPixels.rowBytes());

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
GrTexture* GrSWMaskHelper::DrawShapeMaskToTexture(GrTextureProvider* texProvider,
                                                  const GrShape& shape,
                                                  const SkIRect& resultBounds,
                                                  bool antiAlias,
                                                  const SkMatrix* matrix) {
    GrSWMaskHelper helper(texProvider);

    if (!helper.init(resultBounds, matrix)) {
        return nullptr;
    }

    helper.drawShape(shape, SkRegion::kReplace_Op, antiAlias, 0xFF);

    GrTexture* texture(helper.createTexture());
    if (!texture) {
        return nullptr;
    }

    helper.toTexture(texture);

    return texture;
}

void GrSWMaskHelper::DrawToTargetWithShapeMask(GrTexture* texture,
                                               GrDrawContext* drawContext,
                                               const GrPaint* paint,
                                               const GrUserStencilSettings* userStencilSettings,
                                               const GrClip& clip,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkIRect& rect) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    SkRect dstRect = SkRect::MakeLTRB(SK_Scalar1 * rect.fLeft,
                                      SK_Scalar1 * rect.fTop,
                                      SK_Scalar1 * rect.fRight,
                                      SK_Scalar1 * rect.fBottom);

    // We use device coords to compute the texture coordinates. We take the device coords and apply
    // a translation so that the top-left of the device bounds maps to 0,0, and then a scaling
    // matrix to normalized coords.
    SkMatrix maskMatrix;
    maskMatrix.setIDiv(texture->width(), texture->height());
    maskMatrix.preTranslate(SkIntToScalar(-rect.fLeft), SkIntToScalar(-rect.fTop));

    GrPipelineBuilder pipelineBuilder(*paint, drawContext->mustUseHWAA(*paint));
    pipelineBuilder.setUserStencil(userStencilSettings);

    pipelineBuilder.addCoverageFragmentProcessor(
                         GrSimpleTextureEffect::Make(texture,
                                                     maskMatrix,
                                                     GrTextureParams::kNone_FilterMode,
                                                     kDevice_GrCoordSet));

    SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, SkMatrix::I(),
                                                                        dstRect, nullptr, &invert));
    drawContext->drawBatch(pipelineBuilder, clip, batch);
}
