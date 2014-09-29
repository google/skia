/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "SkAutoKern.h"
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

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
}

void GrStencilAndCoverTextContext::drawText(const GrPaint& paint,
                                            const SkPaint& skPaint,
                                            const char text[],
                                            size_t byteLength,
                                            SkScalar x, SkScalar y) {
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

    this->init(paint, skPaint, byteLength, kMaxAccuracy_RenderMode);

    // Transform our starting point.
    if (fNeedsDeviceSpaceGlyphs) {
        SkPoint loc;
        fContextInitialMatrix.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    fTransformType = GrPathRendering::kTranslate_PathTransformType;

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
        fx += SkFixedMul_portable(autokern.adjust(glyph), fixedSizeRatio);
        if (glyph.fWidth) {
            this->appendGlyph(glyph.getGlyphID(), SkFixedToScalar(fx), SkFixedToScalar(fy));
        }

        fx += SkFixedMul_portable(glyph.fAdvanceX, fixedSizeRatio);
        fy += SkFixedMul_portable(glyph.fAdvanceY, fixedSizeRatio);
    }

    this->finish();
}

void GrStencilAndCoverTextContext::drawPosText(const GrPaint& paint,
                                               const SkPaint& skPaint,
                                               const char text[],
                                               size_t byteLength,
                                               const SkScalar pos[],
                                               SkScalar constY,
                                               int scalarsPerPosition) {
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

    const float textTranslateY = (1 == scalarsPerPosition ? constY : 0);
    this->init(paint, skPaint, byteLength, kMaxPerformance_RenderMode, textTranslateY);

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
        SkTextMapStateProc tmsProc(SkMatrix::I(), 0, scalarsPerPosition);
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
                                        SkScalar textTranslateY) {
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
        SkASSERT(0 == textTranslateY); // TODO: Handle textTranslateY in device-space usecase.

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
        textMatrix.setTranslate(0, textTranslateY);
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

