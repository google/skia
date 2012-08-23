/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSWMaskHelper.h"
#include "GrDrawState.h"
#include "GrGpu.h"

// TODO: try to remove this #include
#include "GrContext.h"

namespace {
/*
 * Convert a boolean operation into a transfer mode code
 */
SkXfermode::Mode op_to_mode(SkRegion::Op op) {

    static const SkXfermode::Mode modeMap[] = {
        SkXfermode::kDstOut_Mode,   // kDifference_Op
        SkXfermode::kMultiply_Mode, // kIntersect_Op
        SkXfermode::kSrcOver_Mode,  // kUnion_Op
        SkXfermode::kXor_Mode,      // kXOR_Op
        SkXfermode::kClear_Mode,    // kReverseDifference_Op
        SkXfermode::kSrc_Mode,      // kReplace_Op
    };

    return modeMap[op];
}

////////////////////////////////////////////////////////////////////////////////
SkPath::FillType gr_fill_to_sk_fill(GrPathFill fill) {
    switch (fill) {
        case kWinding_GrPathFill:
            return SkPath::kWinding_FillType;
        case kEvenOdd_GrPathFill:
            return SkPath::kEvenOdd_FillType;
        case kInverseWinding_GrPathFill:
            return SkPath::kInverseWinding_FillType;
        case kInverseEvenOdd_GrPathFill:
            return SkPath::kInverseEvenOdd_FillType;
        default:
            GrCrash("Unexpected fill.");
            return SkPath::kWinding_FillType;
    }
}

}

/**
 * Draw a single rect element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const GrRect& rect, SkRegion::Op op,
                          bool antiAlias, uint8_t alpha) {
    SkPaint paint;

    SkXfermode* mode = SkXfermode::Create(op_to_mode(op));

    paint.setXfermode(mode);
    paint.setAntiAlias(antiAlias);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawRect(rect, paint);

    SkSafeUnref(mode);
}

/**
 * Draw a single path element of the clip stack into the accumulation bitmap
 */
void GrSWMaskHelper::draw(const SkPath& path, SkRegion::Op op,
                          GrPathFill fill, bool antiAlias, uint8_t alpha) {

    SkPaint paint;
    SkPath tmpPath;
    const SkPath* pathToDraw = &path;
    if (kHairLine_GrPathFill == fill) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1);
    } else {
        paint.setStyle(SkPaint::kFill_Style);
        SkPath::FillType skfill = gr_fill_to_sk_fill(fill);
        if (skfill != pathToDraw->getFillType()) {
            tmpPath = *pathToDraw;
            tmpPath.setFillType(skfill);
            pathToDraw = &tmpPath;
        }
    }
    SkXfermode* mode = SkXfermode::Create(op_to_mode(op));

    paint.setXfermode(mode);
    paint.setAntiAlias(antiAlias);
    paint.setColor(SkColorSetARGB(alpha, alpha, alpha, alpha));

    fDraw.drawPath(*pathToDraw, paint);

    SkSafeUnref(mode);
}

bool GrSWMaskHelper::init(const GrIRect& resultBounds,
                          const GrMatrix* matrix) {
    if (NULL != matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }

    // Now translate so the bound's UL corner is at the origin
    fMatrix.postTranslate(-resultBounds.fLeft * SK_Scalar1,
                          -resultBounds.fTop * SK_Scalar1);
    GrIRect bounds = GrIRect::MakeWH(resultBounds.width(),
                                     resultBounds.height());

    fBM.setConfig(SkBitmap::kA8_Config, bounds.fRight, bounds.fBottom);
    if (!fBM.allocPixels()) {
        return false;
    }
    sk_bzero(fBM.getPixels(), fBM.getSafeSize());

    sk_bzero(&fDraw, sizeof(fDraw));
    fRasterClip.setRect(bounds);
    fDraw.fRC    = &fRasterClip;
    fDraw.fClip  = &fRasterClip.bwRgn();
    fDraw.fMatrix = &fMatrix;
    fDraw.fBitmap = &fBM;
    return true;
}

/**
 * Get a texture (from the texture cache) of the correct size & format.
 * Return true on success; false on failure.
 */
bool GrSWMaskHelper::getTexture(GrAutoScratchTexture* texture) {
    GrTextureDesc desc;
    desc.fWidth = fBM.width();
    desc.fHeight = fBM.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    texture->set(fContext, desc);
    return NULL != texture->texture();
}

/**
 * Move the result of the software mask generation back to the gpu
 */
void GrSWMaskHelper::toTexture(GrTexture *texture, uint8_t alpha) {
    SkAutoLockPixels alp(fBM);

    // The destination texture is almost always larger than "fBM". Clear
    // it appropriately so we don't get mask artifacts outside of the path's
    // bounding box

    // "texture" needs to be installed as the render target for the clear
    // and the texture upload but cannot remain the render target upon
    // return. Callers typically use it as a texture and it would then
    // be both source and dest.
    GrDrawState::AutoRenderTargetRestore artr(fContext->getGpu()->drawState(),
                                              texture->asRenderTarget());

    fContext->getGpu()->clear(NULL, SkColorSetARGB(alpha, alpha, alpha, alpha));

    texture->writePixels(0, 0, fBM.width(), fBM.height(),
                         kAlpha_8_GrPixelConfig,
                         fBM.getPixels(), fBM.rowBytes());
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Software rasterizes path to A8 mask (possibly using the context's matrix)
 * and uploads the result to a scratch texture. Returns the resulting
 * texture on success; NULL on failure.
 */
GrTexture* GrSWMaskHelper::DrawPathMaskToTexture(GrContext* context,
                                                 const SkPath& path,
                                                 const GrIRect& resultBounds,
                                                 GrPathFill fill,
                                                 bool antiAlias,
                                                 GrMatrix* matrix) {
    GrAutoScratchTexture ast;

    GrSWMaskHelper helper(context);

    if (!helper.init(resultBounds, matrix)) {
        return NULL;
    }

    helper.draw(path, SkRegion::kReplace_Op, fill, antiAlias, 0xFF);

    if (!helper.getTexture(&ast)) {
        return NULL;
    }

    helper.toTexture(ast.texture(), 0x00);

    return ast.detach();
}

void GrSWMaskHelper::DrawToTargetWithPathMask(GrTexture* texture,
                                              GrDrawTarget* target,
                                              const GrIRect& rect) {
    GrDrawState* drawState = target->drawState();

    GrDrawTarget::AutoDeviceCoordDraw adcd(target);
    if (!adcd.succeeded()) {
        return;
    }
    enum {
        // the SW path renderer shares this stage with glyph
        // rendering (kGlyphMaskStage in GrBatchedTextContext)
        kPathMaskStage = GrPaint::kTotalStages,
    };
    GrAssert(!drawState->isStageEnabled(kPathMaskStage));
    drawState->sampler(kPathMaskStage)->reset();
    drawState->createTextureEffect(kPathMaskStage, texture);
    GrScalar w = GrIntToScalar(rect.width());
    GrScalar h = GrIntToScalar(rect.height());
    GrRect maskRect = GrRect::MakeWH(w / texture->width(),
                                     h / texture->height());

    const GrRect* srcRects[GrDrawState::kNumStages] = { NULL };
    srcRects[kPathMaskStage] = &maskRect;
    GrRect dstRect = GrRect::MakeLTRB(
                            SK_Scalar1 * rect.fLeft,
                            SK_Scalar1 * rect.fTop,
                            SK_Scalar1 * rect.fRight,
                            SK_Scalar1 * rect.fBottom);
    target->drawRect(dstRect, NULL, srcRects, NULL);
    drawState->disableStage(kPathMaskStage);
}

