/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrAtlasTextContext.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "GrResourceProvider.h"
#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkTextMapStateProc.h"
#include "SkTextFormatParams.h"

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(GrContext* context,
                                                           GrDrawContext* drawContext,
                                                           const SkSurfaceProps& surfaceProps)
    : GrTextContext(context, drawContext, surfaceProps)
    , fStroke(SkStrokeRec::kFill_InitStyle)
    , fQueuedGlyphCount(0)
    , fFallbackGlyphsIdx(kGlyphBufferSize) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create(GrContext* context, GrDrawContext* drawContext,
                                     const SkSurfaceProps& surfaceProps) {
    GrStencilAndCoverTextContext* textContext = SkNEW_ARGS(GrStencilAndCoverTextContext,
                                                           (context, drawContext, surfaceProps));
    textContext->fFallbackTextContext = GrAtlasTextContext::Create(context, drawContext, surfaceProps);

    return textContext;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
}

bool GrStencilAndCoverTextContext::canDraw(const GrRenderTarget* rt,
                                           const GrClip& clip,
                                           const GrPaint& paint,
                                           const SkPaint& skPaint,
                                           const SkMatrix& viewMatrix) {
    if (skPaint.getRasterizer()) {
        return false;
    }
    if (skPaint.getMaskFilter()) {
        return false;
    }
    if (SkPathEffect* pe = skPaint.getPathEffect()) {
        if (pe->asADash(NULL) != SkPathEffect::kDash_DashType) {
            return false;
        }
    }

    // No hairlines unless we can map the 1 px width to the object space.
    if (skPaint.getStyle() == SkPaint::kStroke_Style
        && skPaint.getStrokeWidth() == 0
        && viewMatrix.hasPerspective()) {
        return false;
    }

    // No color bitmap fonts.
    SkScalerContext::Rec    rec;
    SkScalerContext::MakeRec(skPaint, &fSurfaceProps, NULL, &rec);
    return rec.getFormat() != SkMask::kARGB32_Format;
}

void GrStencilAndCoverTextContext::onDrawText(GrRenderTarget* rt,
                                              const GrClip& clip,
                                              const GrPaint& paint,
                                              const SkPaint& skPaint,
                                              const SkMatrix& viewMatrix,
                                              const char text[],
                                              size_t byteLength,
                                              SkScalar x, SkScalar y,
                                              const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != NULL);

    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    // This is the slow path, mainly used by Skia unit tests.  The other
    // backends (8888, gpu, ...) use device-space dependent glyph caches. In
    // order to match the glyph positions that the other code paths produce, we
    // must also use device-space dependent glyph cache. This has the
    // side-effect that the glyph shape outline will be in device-space,
    // too. This in turn has the side-effect that NVPR can not stroke the paths,
    // as the stroke in NVPR is defined in object-space.
    // NOTE: here we have following coincidence that works at the moment:
    // - When using the device-space glyphs, the transforms we pass to NVPR
    // instanced drawing are the global transforms, and the view transform is
    // identity. NVPR can not use non-affine transforms in the instanced
    // drawing. This is taken care of by SkDraw::ShouldDrawTextAsPaths since it
    // will turn off the use of device-space glyphs when perspective transforms
    // are in use.

    this->init(rt, clip, paint, skPaint, byteLength, kMaxAccuracy_RenderMode, viewMatrix,
               regionClipBounds);

    // Transform our starting point.
    if (fUsingDeviceSpaceGlyphs) {
        SkPoint loc;
        fContextInitialMatrix.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    const char* stop = text + byteLength;

    // Measure first if needed.
    if (fSkPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkFixed    stopX = 0;
        SkFixed    stopY = 0;

        const char* textPtr = text;
        while (textPtr < stop) {
            // We don't need x, y here, since all subpixel variants will have the
            // same advance.
            const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &textPtr, 0, 0);

            stopX += glyph.fAdvanceX;
            stopY += glyph.fAdvanceY;
        }
        SkASSERT(textPtr == stop);

        SkScalar alignX = SkFixedToScalar(stopX) * fTextRatio;
        SkScalar alignY = SkFixedToScalar(stopY) * fTextRatio;

        if (fSkPaint.getTextAlign() == SkPaint::kCenter_Align) {
            alignX = SkScalarHalf(alignX);
            alignY = SkScalarHalf(alignY);
        }

        x -= alignX;
        y -= alignY;
    }

    SkAutoKern autokern;

    SkFixed fixedSizeRatio = SkScalarToFixed(fTextRatio);

    SkFixed fx = SkScalarToFixed(x);
    SkFixed fy = SkScalarToFixed(y);
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &text, 0, 0);
        fx += SkFixedMul(autokern.adjust(glyph), fixedSizeRatio);
        if (glyph.fWidth) {
            this->appendGlyph(glyph, SkPoint::Make(SkFixedToScalar(fx), SkFixedToScalar(fy)));
        }

        fx += SkFixedMul(glyph.fAdvanceX, fixedSizeRatio);
        fy += SkFixedMul(glyph.fAdvanceY, fixedSizeRatio);
    }

    this->finish();
}

void GrStencilAndCoverTextContext::onDrawPosText(GrRenderTarget* rt,
                                                 const GrClip& clip,
                                                 const GrPaint& paint,
                                                 const SkPaint& skPaint,
                                                 const SkMatrix& viewMatrix,
                                                 const char text[],
                                                 size_t byteLength,
                                                 const SkScalar pos[],
                                                 int scalarsPerPosition,
                                                 const SkPoint& offset,
                                                 const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0/* || fRC->isEmpty()*/) {
        return;
    }

    // This is the fast path.  Here we do not bake in the device-transform to
    // the glyph outline or the advances. This is because we do not need to
    // position the glyphs at all, since the caller has done the positioning.
    // The positioning is based on SkPaint::measureText of individual
    // glyphs. That already uses glyph cache without device transforms. Device
    // transform is not part of SkPaint::measureText API, and thus we use the
    // same glyphs as what were measured.

    this->init(rt, clip, paint, skPaint, byteLength, kMaxPerformance_RenderMode, viewMatrix,
               regionClipBounds);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    const char* stop = text + byteLength;

    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);
    SkTextAlignProc alignProc(fSkPaint.getTextAlign());
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &text, 0, 0);
        if (glyph.fWidth) {
            SkPoint tmsLoc;
            tmsProc(pos, &tmsLoc);
            SkPoint loc;
            alignProc(tmsLoc, glyph, &loc);

            this->appendGlyph(glyph, loc);
        }
        pos += scalarsPerPosition;
    }

    this->finish();
}

static GrPathRange* get_gr_glyphs(GrContext* ctx,
                                  const SkTypeface* typeface,
                                  const SkDescriptor* desc,
                                  const GrStrokeInfo& stroke) {

    static const GrUniqueKey::Domain kPathGlyphDomain = GrUniqueKey::GenerateDomain();
    int strokeDataCount = stroke.computeUniqueKeyFragmentData32Cnt();
    GrUniqueKey glyphKey;
    GrUniqueKey::Builder builder(&glyphKey, kPathGlyphDomain, 2 + strokeDataCount);
    reinterpret_cast<uint32_t&>(builder[0]) = desc ? desc->getChecksum() : 0;
    reinterpret_cast<uint32_t&>(builder[1]) = typeface ? typeface->uniqueID() : 0;
    if (strokeDataCount > 0) {
        stroke.asUniqueKeyFragment(&builder[2]);
    }
    builder.finish();

    SkAutoTUnref<GrPathRange> glyphs(
        static_cast<GrPathRange*>(
            ctx->resourceProvider()->findAndRefResourceByUniqueKey(glyphKey)));
    if (NULL == glyphs) {
        glyphs.reset(ctx->resourceProvider()->createGlyphs(typeface, desc, stroke));
        ctx->resourceProvider()->assignUniqueKeyToResource(glyphKey, glyphs);
    } else {
        SkASSERT(NULL == desc || glyphs->isEqualTo(*desc));
    }

    return glyphs.detach();
}

void GrStencilAndCoverTextContext::init(GrRenderTarget* rt,
                                        const GrClip& clip,
                                        const GrPaint& paint,
                                        const SkPaint& skPaint,
                                        size_t textByteLength,
                                        RenderMode renderMode,
                                        const SkMatrix& viewMatrix,
                                        const SkIRect& regionClipBounds) {
    fClip = clip;

    fRenderTarget.reset(SkRef(rt));

    fRegionClipBounds = regionClipBounds;
    fClip.getConservativeBounds(fRenderTarget->width(), fRenderTarget->height(), &fClipRect);

    fPaint = paint;
    fSkPaint = skPaint;

    fContextInitialMatrix = viewMatrix;
    fViewMatrix = viewMatrix;
    fLocalMatrix = SkMatrix::I();

    const bool otherBackendsWillDrawAsPaths =
        SkDraw::ShouldDrawTextAsPaths(skPaint, fContextInitialMatrix);

    fUsingDeviceSpaceGlyphs = !otherBackendsWillDrawAsPaths &&
                              kMaxAccuracy_RenderMode == renderMode &&
                              SkToBool(fContextInitialMatrix.getType() &
                                       (SkMatrix::kScale_Mask | SkMatrix::kAffine_Mask));

    if (fUsingDeviceSpaceGlyphs) {
        // SkDraw::ShouldDrawTextAsPaths takes care of perspective transforms.
        SkASSERT(!fContextInitialMatrix.hasPerspective());

        // The whole shape (including stroke) will be baked into the glyph outlines. Make
        // NVPR just fill the baked shapes.
        fStroke = GrStrokeInfo(SkStrokeRec::kFill_InitStyle);

        fTextRatio = fTextInverseRatio = 1.0f;

        // Glyphs loaded by GPU path rendering have an inverted y-direction.
        SkMatrix m;
        m.setScale(1, -1);
        fViewMatrix = m;

        // Post-flip the initial matrix so we're left with just the flip after
        // the paint preConcats the inverse.
        m = fContextInitialMatrix;
        m.postScale(1, -1);
        if (!m.invert(&fLocalMatrix)) {
            SkDebugf("Not invertible!\n");
            return;
        }

        fGlyphCache = fSkPaint.detachCache(&fSurfaceProps, &fContextInitialMatrix,
                                           true /*ignoreGamma*/);
        fGlyphs = get_gr_glyphs(fContext, fGlyphCache->getScalerContext()->getTypeface(),
                                &fGlyphCache->getDescriptor(), fStroke);
    } else {
        // Don't bake strokes into the glyph outlines. We will stroke the glyphs
        // using the GPU instead. This is the fast path.
        fStroke = GrStrokeInfo(fSkPaint);
        fSkPaint.setStyle(SkPaint::kFill_Style);

        if (fStroke.isHairlineStyle()) {
            // Approximate hairline stroke.
            SkScalar strokeWidth = SK_Scalar1 /
                (SkVector::Make(fContextInitialMatrix.getScaleX(),
                                fContextInitialMatrix.getSkewY()).length());
            fStroke.setStrokeStyle(strokeWidth, false /*strokeAndFill*/);

        } else if (fSkPaint.isFakeBoldText() &&
#ifdef SK_USE_FREETYPE_EMBOLDEN
                   kMaxPerformance_RenderMode == renderMode &&
#endif
                   SkStrokeRec::kStroke_Style != fStroke.getStyle()) {

            // Instead of baking fake bold into the glyph outlines, do it with the GPU stroke.
            SkScalar fakeBoldScale = SkScalarInterpFunc(fSkPaint.getTextSize(),
                                                        kStdFakeBoldInterpKeys,
                                                        kStdFakeBoldInterpValues,
                                                        kStdFakeBoldInterpLength);
            SkScalar extra = SkScalarMul(fSkPaint.getTextSize(), fakeBoldScale);
            fStroke.setStrokeStyle(fStroke.needToApply() ? fStroke.getWidth() + extra : extra,
                                   true /*strokeAndFill*/);

            fSkPaint.setFakeBoldText(false);
        }

        bool canUseRawPaths;
        if (!fStroke.isDashed() && (otherBackendsWillDrawAsPaths ||
                                    kMaxPerformance_RenderMode == renderMode)) {
            // We can draw the glyphs from canonically sized paths.
            fTextRatio = fSkPaint.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
            fTextInverseRatio = SkPaint::kCanonicalTextSizeForPaths / fSkPaint.getTextSize();

            // Compensate for the glyphs being scaled by fTextRatio.
            if (!fStroke.isFillStyle()) {
                fStroke.setStrokeStyle(fStroke.getWidth() / fTextRatio,
                                       SkStrokeRec::kStrokeAndFill_Style == fStroke.getStyle());
            }

            fSkPaint.setLinearText(true);
            fSkPaint.setLCDRenderText(false);
            fSkPaint.setAutohinted(false);
            fSkPaint.setHinting(SkPaint::kNo_Hinting);
            fSkPaint.setSubpixelText(true);
            fSkPaint.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));

            canUseRawPaths = SK_Scalar1 == fSkPaint.getTextScaleX() &&
                             0 == fSkPaint.getTextSkewX() &&
                             !fSkPaint.isFakeBoldText() &&
                             !fSkPaint.isVerticalText();
        } else {
            fTextRatio = fTextInverseRatio = 1.0f;
            canUseRawPaths = false;
        }

        SkMatrix textMatrix;
        // Glyphs loaded by GPU path rendering have an inverted y-direction.
        textMatrix.setScale(fTextRatio, -fTextRatio);
        fViewMatrix.preConcat(textMatrix);
        fLocalMatrix = textMatrix;

        fGlyphCache = fSkPaint.detachCache(&fSurfaceProps, NULL, true /*ignoreGamma*/);
        fGlyphs = canUseRawPaths ?
                      get_gr_glyphs(fContext, fSkPaint.getTypeface(), NULL, fStroke) :
                      get_gr_glyphs(fContext, fGlyphCache->getScalerContext()->getTypeface(),
                                    &fGlyphCache->getDescriptor(), fStroke);
    }

}

bool GrStencilAndCoverTextContext::mapToFallbackContext(SkMatrix* inverse) {
    // The current view matrix is flipped because GPU path rendering glyphs have an
    // inverted y-direction. Unflip the view matrix for the fallback context. If using
    // device-space glyphs, we'll also need to restore the original view matrix since
    // we moved that transfomation into our local glyph cache for this scenario. Also
    // track the inverse operation so the caller can unmap the paint and glyph positions.
    if (fUsingDeviceSpaceGlyphs) {
        fViewMatrix = fContextInitialMatrix;
        if (!fContextInitialMatrix.invert(inverse)) {
            return false;
        }
        inverse->preScale(1, -1);
    } else {
        inverse->setScale(1, -1);
        const SkMatrix& unflip = *inverse; // unflip is equal to its own inverse.
        fViewMatrix.preConcat(unflip);
    }
    return true;
}

inline void GrStencilAndCoverTextContext::appendGlyph(const SkGlyph& glyph, const SkPoint& pos) {
    if (fQueuedGlyphCount >= fFallbackGlyphsIdx) {
        SkASSERT(fQueuedGlyphCount == fFallbackGlyphsIdx);
        this->flush();
    }

    // Stick the glyphs we can't draw at the end of the buffer, growing backwards.
    int index = (SkMask::kARGB32_Format == glyph.fMaskFormat) ?
                --fFallbackGlyphsIdx : fQueuedGlyphCount++;

    fGlyphIndices[index] = glyph.getGlyphID();
    fGlyphPositions[index].set(fTextInverseRatio * pos.x(), -fTextInverseRatio * pos.y());
}

static const SkScalar* get_xy_scalar_array(const SkPoint* pointArray) {
    GR_STATIC_ASSERT(2 * sizeof(SkScalar) == sizeof(SkPoint));
    GR_STATIC_ASSERT(0 == offsetof(SkPoint, fX));

    return &pointArray[0].fX;
}

void GrStencilAndCoverTextContext::flush() {
    if (fQueuedGlyphCount > 0) {
        SkAutoTUnref<GrPathProcessor> pp(GrPathProcessor::Create(fPaint.getColor(),
                                                                 fViewMatrix,
                                                                 fLocalMatrix));

        // We should only be flushing about once every run.  However, if this impacts performance
        // we could move the creation of the GrPipelineBuilder earlier.
        GrPipelineBuilder pipelineBuilder(fPaint, fRenderTarget, fClip);
        SkASSERT(fRenderTarget->isStencilBufferMultisampled() || !fPaint.isAntiAlias());
        pipelineBuilder.setState(GrPipelineBuilder::kHWAntialias_Flag, fPaint.isAntiAlias());

        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
                                     kZero_StencilOp,
                                     kZero_StencilOp,
                                     kNotEqual_StencilFunc,
                                     0xffff,
                                     0x0000,
                                     0xffff);

        *pipelineBuilder.stencil() = kStencilPass;

        SkASSERT(0 == fQueuedGlyphCount);
        SkASSERT(kGlyphBufferSize == fFallbackGlyphsIdx);

        fDrawContext->drawPaths(&pipelineBuilder, pp, fGlyphs,
                                fGlyphIndices, GrPathRange::kU16_PathIndexType,
                                get_xy_scalar_array(fGlyphPositions),
                                GrPathRendering::kTranslate_PathTransformType,
                                fQueuedGlyphCount, GrPathRendering::kWinding_FillType);

        fQueuedGlyphCount = 0;
    }

    if (fFallbackGlyphsIdx < kGlyphBufferSize) {
        int fallbackGlyphCount = kGlyphBufferSize - fFallbackGlyphsIdx;

        GrPaint paintFallback(fPaint);

        SkPaint skPaintFallback(fSkPaint);
        if (!fUsingDeviceSpaceGlyphs) {
            fStroke.applyToPaint(&skPaintFallback);
        }
        skPaintFallback.setTextAlign(SkPaint::kLeft_Align); // Align has already been accounted for.
        skPaintFallback.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        SkMatrix inverse;
        if (this->mapToFallbackContext(&inverse)) {
            inverse.mapPoints(&fGlyphPositions[fFallbackGlyphsIdx], fallbackGlyphCount);
        }

        fFallbackTextContext->drawPosText(fRenderTarget, fClip, paintFallback, skPaintFallback,
                                          fViewMatrix, (char*)&fGlyphIndices[fFallbackGlyphsIdx],
                                          2 * fallbackGlyphCount,
                                          get_xy_scalar_array(&fGlyphPositions[fFallbackGlyphsIdx]),
                                          2, SkPoint::Make(0, 0), fRegionClipBounds);

        fFallbackGlyphsIdx = kGlyphBufferSize;
    }
}

void GrStencilAndCoverTextContext::finish() {
    this->flush();

    fGlyphs->unref();
    fGlyphs = NULL;

    SkGlyphCache::AttachCache(fGlyphCache);
    fGlyphCache = NULL;

    fViewMatrix = fContextInitialMatrix;
}

