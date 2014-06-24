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
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkTextMapStateProc.h"

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

    fGlyphTransform = fContext->getMatrix();

    this->init(paint, skPaint, byteLength);

    SkMatrix* glyphCacheTransform = NULL;
    // Transform our starting point.
    if (fNeedsDeviceSpaceGlyphs) {
        SkPoint loc;
        fGlyphTransform.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
        glyphCacheTransform = &fGlyphTransform;
    }

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();
    SkAutoGlyphCache autoCache(fSkPaint, &fDeviceProperties, glyphCacheTransform);
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
            // We don't need x, y here, since all subpixel variants will have the
            // same advance.
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

    SkFixed fixedSizeRatio = SkScalarToFixed(fTextRatio);

    SkFixed fx = SkScalarToFixed(x);
    SkFixed fy = SkScalarToFixed(y);
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);
        fx += SkFixedMul_portable(autokern.adjust(glyph), fixedSizeRatio);
        if (glyph.fWidth) {
            this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                            glyph.getSubXFixed(),
                                            glyph.getSubYFixed()),
                              SkPoint::Make(
                                  SkFixedToScalar(fx),
                                  SkFixedToScalar(fy)),
                              strike,
                              scaler);
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
    fGlyphTransform.reset();

    this->init(paint, skPaint, byteLength);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache* cache = autoCache.getCache();
    GrFontScaler* scaler = GetGrFontScaler(cache);
    GrTextStrike* strike =
        fContext->getFontCache()->getStrike(scaler, true);

    const char* stop = text + byteLength;
    SkTextAlignProcScalar alignProc(fSkPaint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), constY, scalarsPerPosition);

    if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
        while (text < stop) {
            SkPoint loc;
            tmsProc(pos, &loc);
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);
            if (glyph.fWidth) {
                this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                glyph.getSubXFixed(),
                                                glyph.getSubYFixed()),
                                  loc,
                                  strike,
                                  scaler);
            }
            pos += scalarsPerPosition;
        }
    } else {
        while (text < stop) {
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);
            if (glyph.fWidth) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                glyph.getSubXFixed(),
                                                glyph.getSubYFixed()),
                                  loc,
                                  strike,
                                  scaler);

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

void GrStencilAndCoverTextContext::init(const GrPaint& paint,
                                        const SkPaint& skPaint,
                                        size_t textByteLength) {
    GrTextContext::init(paint, skPaint);

    bool otherBackendsWillDrawAsPaths =
        SkDraw::ShouldDrawTextAsPaths(skPaint, fContext->getMatrix());

    if (otherBackendsWillDrawAsPaths) {
        // This is to reproduce SkDraw::drawText_asPaths glyph positions.
        fSkPaint.setLinearText(true);
        fTextRatio = fSkPaint.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
        fSkPaint.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));
        if (fSkPaint.getStyle() != SkPaint::kFill_Style) {
            // Compensate the glyphs being scaled up by fTextRatio by scaling the
            // stroke down.
            fSkPaint.setStrokeWidth(fSkPaint.getStrokeWidth() / fTextRatio);
        }
        fNeedsDeviceSpaceGlyphs = false;
    } else {
        fTextRatio = 1.0f;
        fNeedsDeviceSpaceGlyphs = (fGlyphTransform.getType() &
            (SkMatrix::kScale_Mask | SkMatrix::kAffine_Mask)) != 0;
        // SkDraw::ShouldDrawTextAsPaths takes care of perspective transforms.
        SkASSERT(!fGlyphTransform.hasPerspective());
        if (fNeedsDeviceSpaceGlyphs) {
            fPaint.localCoordChangeInverse(fGlyphTransform);
            fContext->setIdentityMatrix();
        }
    }

    fStroke = SkStrokeRec(fSkPaint);

    if (fNeedsDeviceSpaceGlyphs) {
        // The whole shape is baked into the glyph. Make NVPR just fill the
        // baked shape.
        fStroke.setStrokeStyle(-1, false);
    } else {
        if (fSkPaint.getStrokeWidth() == 0.0f) {
            if (fSkPaint.getStyle() == SkPaint::kStrokeAndFill_Style) {
                fStroke.setStrokeStyle(-1, false);
            } else if (fSkPaint.getStyle() == SkPaint::kStroke_Style) {
                // Approximate hairline stroke.
                const SkMatrix& ctm = fContext->getMatrix();
                SkScalar strokeWidth = SK_Scalar1 /
                    (fTextRatio * SkVector::Make(ctm.getScaleX(), ctm.getSkewY()).length());
                fStroke.setStrokeStyle(strokeWidth, false);
            }
        }

        // Make glyph cache produce paths geometry for fill. We will stroke them
        // by passing fStroke to drawPath. This is the fast path.
        fSkPaint.setStyle(SkPaint::kFill_Style);
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
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return;
    }

    if (scaler->getGlyphPath(glyph->glyphID(), &fTmpPath)) {
        if (!fTmpPath.isEmpty()) {
            *fPaths.append() = fContext->createPath(fTmpPath, fStroke);
            SkMatrix* t = fTransforms.append();
            t->setTranslate(pos.fX, pos.fY);
            t->preScale(fTextRatio, fTextRatio);
        }
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
    if (fNeedsDeviceSpaceGlyphs) {
        fContext->setMatrix(fGlyphTransform);
    }
    GrTextContext::finish();
}

