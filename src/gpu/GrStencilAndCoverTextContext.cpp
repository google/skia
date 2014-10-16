/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrBitmapTextContext.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkTextMapStateProc.h"
#include "SkTextFormatParams.h"

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(
    GrContext* context, const SkDeviceProperties& properties)
    : GrTextContext(context, properties)
    , fPendingGlyphCount(0) {
}

GrStencilAndCoverTextContext* GrStencilAndCoverTextContext::Create(GrContext* context,
                                                                 const SkDeviceProperties& props) {
    GrStencilAndCoverTextContext* textContext = SkNEW_ARGS(GrStencilAndCoverTextContext,
                                                           (context, props));
    textContext->fFallbackTextContext = GrBitmapTextContext::Create(context, props);

    return textContext;
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
}

bool GrStencilAndCoverTextContext::canDraw(const SkPaint& paint) {
    if (paint.getRasterizer()) {
        return false;
    }
    if (paint.getMaskFilter()) {
        return false;
    }
    if (paint.getPathEffect()) {
        return false;
    }

    // No hairlines unless we can map the 1 px width to the object space.
    if (paint.getStyle() == SkPaint::kStroke_Style
        && paint.getStrokeWidth() == 0
        && fContext->getMatrix().hasPerspective()) {
        return false;
    }

    // No color bitmap fonts.
    SkScalerContext::Rec    rec;
    SkScalerContext::MakeRec(paint, &fDeviceProperties, NULL, &rec);
    return rec.getFormat() != SkMask::kARGB32_Format;
}

void GrStencilAndCoverTextContext::onDrawPosText(const GrPaint& paint,
                                               const SkPaint& skPaint,
                                               const char text[],
                                               size_t byteLength,
                                               const SkScalar pos[],
                                               int scalarsPerPosition,
                                               const SkPoint& offset) {
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

    this->init(paint, skPaint, byteLength, kMaxPerformance_RenderMode, offset);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    const char* stop = text + byteLength;

    if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
        if (1 == scalarsPerPosition) {
            fTransformType = GrPathRendering::kTranslateX_PathTransformType;
            while (text < stop) {
                const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &text, 0, 0);
                if (glyph.fWidth) {
                    this->appendGlyph(glyph.getGlyphID(), *pos);
                }
                pos++;
            }
        } else {
            SkASSERT(2 == scalarsPerPosition);
            fTransformType = GrPathRendering::kTranslate_PathTransformType;
            while (text < stop) {
                const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &text, 0, 0);
                if (glyph.fWidth) {
                    this->appendGlyph(glyph.getGlyphID(), pos[0], pos[1]);
                }
                pos += 2;
            }
        }
    } else {
        fTransformType = GrPathRendering::kTranslate_PathTransformType;
        SkTextMapStateProc tmsProc(SkMatrix::I(), SkPoint::Make(0, 0), scalarsPerPosition);
        SkTextAlignProcScalar alignProc(fSkPaint.getTextAlign());
        while (text < stop) {
            const SkGlyph& glyph = glyphCacheProc(fGlyphCache, &text, 0, 0);
            if (glyph.fWidth) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                this->appendGlyph(glyph.getGlyphID(), loc.x(), loc.y());
            }
            pos += scalarsPerPosition;
        }
    }

    this->finish();
}

static GrPathRange* get_gr_glyphs(GrContext* ctx,
                                  const SkTypeface* typeface,
                                  const SkDescriptor* desc,
                                  const SkStrokeRec& stroke) {
    static const GrCacheID::Domain gGlyphsDomain = GrCacheID::GenerateDomain();

    GrCacheID::Key key;
    uint64_t* keyData = key.fData64;
    keyData[0] = desc ? desc->getChecksum() : 0;
    keyData[0] = (keyData[0] << 32) | (typeface ? typeface->uniqueID() : 0);
    keyData[1] = GrPath::ComputeStrokeKey(stroke);
    GrResourceKey resourceKey = GrResourceKey(GrCacheID(gGlyphsDomain, key),
                                              GrPathRange::resourceType(), 0);

    SkAutoTUnref<GrPathRange> glyphs(
        static_cast<GrPathRange*>(ctx->findAndRefCachedResource(resourceKey)));
    if (NULL == glyphs || (NULL != desc && !glyphs->isEqualTo(*desc))) {
        glyphs.reset(ctx->getGpu()->pathRendering()->createGlyphs(typeface, desc, stroke));
        ctx->addResourceToCache(resourceKey, glyphs);
    }

    return glyphs.detach();
}

void GrStencilAndCoverTextContext::init(const GrPaint& paint,
                                        const SkPaint& skPaint,
                                        size_t textByteLength,
                                        RenderMode renderMode,
                                        const SkPoint& textTranslate) {
    GrTextContext::init(paint, skPaint);

    fContextInitialMatrix = fContext->getMatrix();

    const bool otherBackendsWillDrawAsPaths =
        SkDraw::ShouldDrawTextAsPaths(skPaint, fContextInitialMatrix);

    fNeedsDeviceSpaceGlyphs = !otherBackendsWillDrawAsPaths &&
                              kMaxAccuracy_RenderMode == renderMode &&
                              SkToBool(fContextInitialMatrix.getType() &
                                       (SkMatrix::kScale_Mask | SkMatrix::kAffine_Mask));

    if (fNeedsDeviceSpaceGlyphs) {
        // SkDraw::ShouldDrawTextAsPaths takes care of perspective transforms.
        SkASSERT(!fContextInitialMatrix.hasPerspective());
        SkASSERT(textTranslate.isZero()); // TODO: Handle textTranslate in device-space usecase.

        fTextRatio = fTextInverseRatio = 1.0f;

        // Glyphs loaded by GPU path rendering have an inverted y-direction.
        SkMatrix m;
        m.setScale(1, -1);
        fContext->setMatrix(m);

        // Post-flip the initial matrix so we're left with just the flip after
        // the paint preConcats the inverse.
        m = fContextInitialMatrix;
        m.postScale(1, -1);
        fPaint.localCoordChangeInverse(m);

        // The whole shape (including stroke) will be baked into the glyph outlines. Make
        // NVPR just fill the baked shapes.
        fGlyphCache = fSkPaint.detachCache(&fDeviceProperties, &fContextInitialMatrix, false);
        fGlyphs = get_gr_glyphs(fContext, fGlyphCache->getScalerContext()->getTypeface(),
                                &fGlyphCache->getDescriptor(),
                                SkStrokeRec(SkStrokeRec::kFill_InitStyle));
    } else {
        // Don't bake strokes into the glyph outlines. We will stroke the glyphs
        // using the GPU instead. This is the fast path.
        SkStrokeRec gpuStroke = SkStrokeRec(fSkPaint);
        fSkPaint.setStyle(SkPaint::kFill_Style);

        if (gpuStroke.isHairlineStyle()) {
            // Approximate hairline stroke.
            SkScalar strokeWidth = SK_Scalar1 /
                (SkVector::Make(fContextInitialMatrix.getScaleX(),
                                fContextInitialMatrix.getSkewY()).length());
            gpuStroke.setStrokeStyle(strokeWidth, false /*strokeAndFill*/);

        } else if (fSkPaint.isFakeBoldText() &&
#ifdef SK_USE_FREETYPE_EMBOLDEN
                   kMaxPerformance_RenderMode == renderMode &&
#endif
                   SkStrokeRec::kStroke_Style != gpuStroke.getStyle()) {

            // Instead of baking fake bold into the glyph outlines, do it with the GPU stroke.
            SkScalar fakeBoldScale = SkScalarInterpFunc(fSkPaint.getTextSize(),
                                                        kStdFakeBoldInterpKeys,
                                                        kStdFakeBoldInterpValues,
                                                        kStdFakeBoldInterpLength);
            SkScalar extra = SkScalarMul(fSkPaint.getTextSize(), fakeBoldScale);
            gpuStroke.setStrokeStyle(gpuStroke.needToApply() ? gpuStroke.getWidth() + extra : extra,
                                     true /*strokeAndFill*/);

            fSkPaint.setFakeBoldText(false);
        }

        bool canUseRawPaths;

        if (otherBackendsWillDrawAsPaths || kMaxPerformance_RenderMode == renderMode) {
            // We can draw the glyphs from canonically sized paths.
            fTextRatio = fSkPaint.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
            fTextInverseRatio = SkPaint::kCanonicalTextSizeForPaths / fSkPaint.getTextSize();

            // Compensate for the glyphs being scaled by fTextRatio.
            if (!gpuStroke.isFillStyle()) {
                gpuStroke.setStrokeStyle(gpuStroke.getWidth() / fTextRatio,
                                         SkStrokeRec::kStrokeAndFill_Style == gpuStroke.getStyle());
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
        textMatrix.setTranslate(textTranslate.x(), textTranslate.y());
        // Glyphs loaded by GPU path rendering have an inverted y-direction.
        textMatrix.preScale(fTextRatio, -fTextRatio);
        fPaint.localCoordChange(textMatrix);
        fContext->concatMatrix(textMatrix);

        fGlyphCache = fSkPaint.detachCache(&fDeviceProperties, NULL, false);
        fGlyphs = canUseRawPaths ?
                      get_gr_glyphs(fContext, fSkPaint.getTypeface(), NULL, gpuStroke) :
                      get_gr_glyphs(fContext, fGlyphCache->getScalerContext()->getTypeface(),
                                    &fGlyphCache->getDescriptor(), gpuStroke);
    }

    fStateRestore.set(fDrawTarget->drawState());

    fDrawTarget->drawState()->setFromPaint(fPaint, fContext->getMatrix(),
                                           fContext->getRenderTarget());

    GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
                                 kZero_StencilOp,
                                 kZero_StencilOp,
                                 kNotEqual_StencilFunc,
                                 0xffff,
                                 0x0000,
                                 0xffff);

    *fDrawTarget->drawState()->stencil() = kStencilPass;

    SkASSERT(0 == fPendingGlyphCount);
}

inline void GrStencilAndCoverTextContext::appendGlyph(uint16_t glyphID, float x) {
    SkASSERT(GrPathRendering::kTranslateX_PathTransformType == fTransformType);

    if (fPendingGlyphCount >= kGlyphBufferSize) {
        this->flush();
    }

    fIndexBuffer[fPendingGlyphCount] = glyphID;
    fTransformBuffer[fPendingGlyphCount] = fTextInverseRatio * x;

    ++fPendingGlyphCount;
}

inline void GrStencilAndCoverTextContext::appendGlyph(uint16_t glyphID, float x, float y) {
    SkASSERT(GrPathRendering::kTranslate_PathTransformType == fTransformType);

    if (fPendingGlyphCount >= kGlyphBufferSize) {
        this->flush();
    }

    fIndexBuffer[fPendingGlyphCount] = glyphID;
    fTransformBuffer[2 * fPendingGlyphCount] = fTextInverseRatio * x;
    fTransformBuffer[2 * fPendingGlyphCount + 1] = -fTextInverseRatio * y;

    ++fPendingGlyphCount;
}

void GrStencilAndCoverTextContext::flush() {
    if (0 == fPendingGlyphCount) {
        return;
    }

    fDrawTarget->drawPaths(fGlyphs, fIndexBuffer, fPendingGlyphCount,
                           fTransformBuffer, fTransformType, SkPath::kWinding_FillType);

    fPendingGlyphCount = 0;
}

void GrStencilAndCoverTextContext::finish() {
    this->flush();

    fGlyphs->unref();
    fGlyphs = NULL;

    SkGlyphCache::AttachCache(fGlyphCache);
    fGlyphCache = NULL;

    fDrawTarget->drawState()->stencil()->setDisabled();
    fStateRestore.set(NULL);
    fContext->setMatrix(fContextInitialMatrix);
    GrTextContext::finish();
}

