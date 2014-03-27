/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverTextContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"

static const int kBaseSCFontSize = 64;
static const int kMaxReservedGlyphs = 64;

GrStencilAndCoverTextContext::GrStencilAndCoverTextContext(
    GrContext* context, const SkDeviceProperties& properties)
    : GrTextContext(context, properties)
    , fStroke(SkStrokeRec::kFill_InitStyle) {
}

GrStencilAndCoverTextContext::~GrStencilAndCoverTextContext() {
}

void GrStencilAndCoverTextContext::drawText(const GrPaint& paint,
                                            const SkPaint& skPaint,
                                            const char text[],
                                            size_t byteLength,
                                            SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(paint, skPaint, byteLength);


    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();
    SkAutoGlyphCache autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache* cache = autoCache.getCache();
    GrFontScaler* scaler = GetGrFontScaler(cache);
    GrTextStrike* strike =
        fContext->getFontCache()->getStrike(scaler, true);

    const char* stop = text + byteLength;

    // Measure first if needed.
    if (fSkPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkFixed    stopX = 0;
        SkFixed    stopY = 0;

        const char* textPtr = text;
        while (textPtr < stop) {
            // don't need x, y here, since all subpixel variants will have the
            // same advance
            const SkGlyph& glyph = glyphCacheProc(cache, &textPtr, 0, 0);

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

    SkFixed fx = SkScalarToFixed(x) + SK_FixedHalf;
    SkFixed fy = SkScalarToFixed(y) + SK_FixedHalf;
    SkFixed fixedSizeRatio = SkScalarToFixed(fTextRatio);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        fx += SkFixedMul_portable(autokern.adjust(glyph), fixedSizeRatio);

        if (glyph.fWidth) {
            this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                            glyph.getSubXFixed(),
                                            glyph.getSubYFixed()),
                              SkPoint::Make(SkFixedToScalar(fx),
                                            SkFixedToScalar(fy)),
                              strike,
                              scaler);
        }

        fx += SkFixedMul_portable(glyph.fAdvanceX, fixedSizeRatio);
        fy += SkFixedMul_portable(glyph.fAdvanceY, fixedSizeRatio);
    }

    this->finish();
}

void GrStencilAndCoverTextContext::drawPosText(const GrPaint& paint, const SkPaint& skPaint,
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

    this->init(paint, skPaint, byteLength);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache* cache = autoCache.getCache();
    GrFontScaler* scaler = GetGrFontScaler(cache);
    GrTextStrike* strike =
        fContext->getFontCache()->getStrike(scaler, true);

    const char*        stop = text + byteLength;

    SkTDArray<const GrPath*> paths;
    SkTDArray<SkMatrix> transforms;

    if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
        while (text < stop) {
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = pos[0];
                SkScalar y = scalarsPerPosition == 1 ? constY : pos[1];
                this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                glyph.getSubXFixed(),
                                                glyph.getSubYFixed()),
                                  SkPoint::Make(x, y),
                                  strike,
                                  scaler);
            }

            pos += scalarsPerPosition;
        }
    } else {
        int alignShift = SkPaint::kCenter_Align == fSkPaint.getTextAlign() ? 1 : 0;
        while (text < stop) {
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = pos[0];
                SkScalar y = scalarsPerPosition == 1 ? constY : pos[1];
                x -= SkFixedToScalar((glyph.fAdvanceX >> alignShift));
                y -= SkFixedToScalar((glyph.fAdvanceY >> alignShift));

                this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                glyph.getSubXFixed(),
                                                glyph.getSubYFixed()),
                                  SkPoint::Make(x, y),
                                  strike,
                                  scaler);
            }
            pos += scalarsPerPosition;
        }
    }

    this->finish();
}

bool GrStencilAndCoverTextContext::canDraw(const SkPaint& paint) {
    return !paint.getRasterizer()
        && !paint.getMaskFilter()
        && (paint.getStyle() == SkPaint::kFill_Style
            || paint.getStrokeWidth() > 0);

}

static bool has_thick_frame(const SkPaint& paint) {
    return paint.getStrokeWidth() > 0 &&
        paint.getStyle() != SkPaint::kFill_Style;
}

void GrStencilAndCoverTextContext::init(const GrPaint& paint,
                                        const SkPaint& skPaint,
                                        size_t textByteLength) {
    GrTextContext::init(paint, skPaint);
    fTextRatio = fSkPaint.getTextSize() / kBaseSCFontSize;
    fSkPaint.setTextSize(SkIntToScalar(kBaseSCFontSize));
    fSkPaint.setLCDRenderText(false);
    fSkPaint.setAutohinted(false);
    fSkPaint.setSubpixelText(true);
    if (has_thick_frame(fSkPaint)) {
        // Compensate the glyphs being scaled up by fTextRatio, by scaling the
        // stroke down.
        fSkPaint.setStrokeWidth(fSkPaint.getStrokeWidth() / fTextRatio);
    }
    fStroke = SkStrokeRec(fSkPaint);

    // Make glyph cache produce paths geometry for fill. We will stroke them
    // by passing fStroke to drawPath.
    fSkPaint.setStyle(SkPaint::kFill_Style);

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

    size_t reserveAmount;
    switch (skPaint.getTextEncoding()) {
        default:
            SkASSERT(false);
        case SkPaint::kUTF8_TextEncoding:
            reserveAmount = textByteLength;
            break;
        case SkPaint::kUTF16_TextEncoding:
            reserveAmount = textByteLength / 2;
            break;
        case SkPaint::kUTF32_TextEncoding:
        case SkPaint::kGlyphID_TextEncoding:
            reserveAmount = textByteLength / 4;
            break;
    }
    fPaths.setReserve(reserveAmount);
    fTransforms.setReserve(reserveAmount);
}

inline void GrStencilAndCoverTextContext::appendGlyph(GrGlyph::PackedID glyphID,
                                                      const SkPoint& pos,
                                                      GrTextStrike* strike,
                                                      GrFontScaler* scaler) {
    GrGlyph* glyph = strike->getGlyph(glyphID, scaler);

    if (scaler->getGlyphPath(glyph->glyphID(), &fTmpPath)) {
        *fPaths.append() = fContext->createPath(fTmpPath, fStroke);
        SkMatrix* t = fTransforms.append();
        t->setTranslate(pos.fX, pos.fY);
        t->preScale(fTextRatio, fTextRatio);
    }
}

void GrStencilAndCoverTextContext::finish() {
    if (fPaths.count() > 0) {
        fDrawTarget->drawPaths(static_cast<size_t>(fPaths.count()),
                               fPaths.begin(), fTransforms.begin(),
                               SkPath::kWinding_FillType, fStroke.getStyle());
        for (int i = 0; i < fPaths.count(); ++i) {
            fPaths[i]->unref();
        }
        if (fPaths.count() > kMaxReservedGlyphs) {
            fPaths.reset();
            fTransforms.reset();
        } else {
            fPaths.rewind();
            fTransforms.rewind();
        }
    }
    fTmpPath.reset();

    fDrawTarget->drawState()->stencil()->setDisabled();
    fStateRestore.set(NULL);
    GrTextContext::finish();
}

