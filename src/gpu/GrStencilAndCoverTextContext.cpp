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

#include "batches/GrDrawPathBatch.h"

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(GrContext* context,
                                                           const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps)
    , fDraw(nullptr)
    , fStroke(SkStrokeRec::kFill_InitStyle) {
}

GrStencilAndCoverTextContext*
GrStencilAndCoverTextContext::Create(GrContext* context, const SkSurfaceProps& surfaceProps) {
    GrStencilAndCoverTextContext* textContext = 
        new GrStencilAndCoverTextContext(context, surfaceProps);
    textContext->fFallbackTextContext = GrAtlasTextContext::Create(context, surfaceProps);

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
        if (pe->asADash(nullptr) != SkPathEffect::kDash_DashType) {
            return false;
        }
    }
    // No hairlines. They would require new paths with customized strokes for every new draw matrix.
    return SkPaint::kStroke_Style != skPaint.getStyle() || 0 != skPaint.getStrokeWidth();
}

void GrStencilAndCoverTextContext::onDrawText(GrDrawContext* dc, GrRenderTarget* rt,
                                              const GrClip& clip,
                                              const GrPaint& paint,
                                              const SkPaint& skPaint,
                                              const SkMatrix& viewMatrix,
                                              const char text[],
                                              size_t byteLength,
                                              SkScalar x, SkScalar y,
                                              const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != nullptr);

    if (text == nullptr || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint, byteLength, viewMatrix, regionClipBounds);

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

    this->finish(dc);
}

void GrStencilAndCoverTextContext::onDrawPosText(GrDrawContext* dc, GrRenderTarget* rt,
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
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0/* || fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint, byteLength, viewMatrix, regionClipBounds);

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

    this->finish(dc);
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
    if (nullptr == glyphs) {
        glyphs.reset(ctx->resourceProvider()->createGlyphs(typeface, desc, stroke));
        ctx->resourceProvider()->assignUniqueKeyToResource(glyphKey, glyphs);
    } else {
        SkASSERT(nullptr == desc || glyphs->isEqualTo(*desc));
    }

    return glyphs.detach();
}

void GrStencilAndCoverTextContext::init(GrRenderTarget* rt,
                                        const GrClip& clip,
                                        const GrPaint& paint,
                                        const SkPaint& skPaint,
                                        size_t textByteLength,
                                        const SkMatrix& viewMatrix,
                                        const SkIRect& regionClipBounds) {
    fClip = clip;

    fRenderTarget.reset(SkRef(rt));

    fRegionClipBounds = regionClipBounds;
    fClip.getConservativeBounds(fRenderTarget->width(), fRenderTarget->height(), &fClipRect);

    fPaint = paint;
    fSkPaint = skPaint;

    // Don't bake strokes into the glyph outlines. We will stroke the glyphs using the GPU instead.
    fStroke = GrStrokeInfo(fSkPaint);
    fSkPaint.setStyle(SkPaint::kFill_Style);

    SkASSERT(!fStroke.isHairlineStyle()); // Hairlines are not supported.

    if (fSkPaint.isFakeBoldText() && SkStrokeRec::kStroke_Style != fStroke.getStyle()) {
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
    if (!fStroke.isDashed()) {
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

    fViewMatrix = viewMatrix;
    fViewMatrix.preScale(fTextRatio, fTextRatio);
    fLocalMatrix.setScale(fTextRatio, fTextRatio);

    fGlyphCache = fSkPaint.detachCache(&fSurfaceProps, nullptr, true /*ignoreGamma*/);
    fGlyphs = canUseRawPaths ?
                  get_gr_glyphs(fContext, fSkPaint.getTypeface(), nullptr, fStroke) :
                  get_gr_glyphs(fContext, fGlyphCache->getScalerContext()->getTypeface(),
                                &fGlyphCache->getDescriptor(), fStroke);
}

inline void GrStencilAndCoverTextContext::appendGlyph(const SkGlyph& glyph, const SkPoint& pos) {
    // Stick the glyphs we can't draw into the fallback arrays.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        fFallbackIndices.push_back(glyph.getGlyphID());
        fFallbackPositions.push_back(pos);
    } else {
        // TODO: infer the reserve count from the text length.
        if (!fDraw) {
            fDraw = GrPathRangeDraw::Create(fGlyphs,
                                            GrPathRendering::kTranslate_PathTransformType,
                                            64);
        }
        float translate[] = { fTextInverseRatio * pos.x(), fTextInverseRatio * pos.y() };
        fDraw->append(glyph.getGlyphID(), translate);
    }
}

void GrStencilAndCoverTextContext::flush(GrDrawContext* dc) {
    if (fDraw) {
        SkASSERT(fDraw->count());

        // We should only be flushing about once every run.  However, if this impacts performance
        // we could move the creation of the GrPipelineBuilder earlier.
        GrPipelineBuilder pipelineBuilder(fPaint, fRenderTarget, fClip);
        SkASSERT(fRenderTarget->isStencilBufferMultisampled() || !fPaint.isAntiAlias());
        pipelineBuilder.setState(GrPipelineBuilder::kHWAntialias_Flag, fPaint.isAntiAlias());

        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
                                     kZero_StencilOp,
                                     kKeep_StencilOp,
                                     kNotEqual_StencilFunc,
                                     0xffff,
                                     0x0000,
                                     0xffff);

        *pipelineBuilder.stencil() = kStencilPass;

        dc->drawPathsFromRange(&pipelineBuilder, fViewMatrix, fLocalMatrix, fPaint.getColor(),
                               fDraw, GrPathRendering::kWinding_FillType);
        fDraw->unref();
        fDraw = nullptr;
    }

    if (fFallbackIndices.count()) {
        SkASSERT(fFallbackPositions.count() == fFallbackIndices.count());

        SkPaint fallbackSkPaint(fSkPaint);
        fStroke.applyToPaint(&fallbackSkPaint);
        if (!fStroke.isFillStyle()) {
            fallbackSkPaint.setStrokeWidth(fStroke.getWidth() * fTextRatio);
        }
        fallbackSkPaint.setTextAlign(SkPaint::kLeft_Align); // Align has already been accounted for.
        fallbackSkPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        // No need for subpixel positioning with bitmap glyphs. TODO: revisit if non-bitmap color
        // glyphs show up and https://code.google.com/p/skia/issues/detail?id=4408 gets resolved.
        fallbackSkPaint.setSubpixelText(false);
        fallbackSkPaint.setTextSize(fSkPaint.getTextSize() * fTextRatio);

        SkMatrix fallbackMatrix(fViewMatrix);
        fallbackMatrix.preScale(fTextInverseRatio, fTextInverseRatio);

        fFallbackTextContext->drawPosText(dc, fRenderTarget, fClip, fPaint, fallbackSkPaint,
                                          fallbackMatrix, (char*)fFallbackIndices.begin(),
                                          sizeof(uint16_t) * fFallbackIndices.count(),
                                          fFallbackPositions[0].asScalars(), 2, SkPoint::Make(0, 0),
                                          fRegionClipBounds);
        fFallbackIndices.reset();
        fFallbackPositions.reset();
    }
}

void GrStencilAndCoverTextContext::finish(GrDrawContext* dc) {
    this->flush(dc);

    SkASSERT(!fDraw);
    SkASSERT(!fFallbackIndices.count());
    SkASSERT(!fFallbackPositions.count());

    fGlyphs->unref();
    fGlyphs = nullptr;

    SkGlyphCache::AttachCache(fGlyphCache);
    fGlyphCache = nullptr;
}
