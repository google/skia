/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextContext.h"
#include "GrAtlas.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrIndexBuffer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkColorPriv.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "effects/GrCustomCoordsTextureEffect.h"

#include "SkAutoKern.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGr.h"

static const int kGlyphCoordsAttributeIndex = 1;

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

GrBitmapTextContext::GrBitmapTextContext(GrContext* context,
                                         const SkDeviceProperties& properties)
                                       : GrTextContext(context, properties) {
    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fMaxVertices = 0;

    fVertexBounds.setLargestInverted();
}

GrBitmapTextContext::~GrBitmapTextContext() {
    this->flushGlyphs();
}

bool GrBitmapTextContext::canDraw(const SkPaint& paint) {
    return !SkDraw::ShouldDrawTextAsPaths(paint, fContext->getMatrix());
}

static inline GrColor skcolor_to_grcolor_nopremultiply(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    return GrColorPackRGBA(r, g, b, 0xff);
}

void GrBitmapTextContext::flushGlyphs() {
    if (NULL == fDrawTarget) {
        return;
    }

    GrDrawState* drawState = fDrawTarget->drawState();
    GrDrawState::AutoRestoreEffects are(drawState);
    drawState->setFromPaint(fPaint, SkMatrix::I(), fContext->getRenderTarget());

    if (fCurrVertex > 0) {
        // setup our sampler state for our text texture/atlas
        SkASSERT(GrIsALIGN4(fCurrVertex));
        SkASSERT(fCurrTexture);
        GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kNone_FilterMode);

        // This effect could be stored with one of the cache objects (atlas?)
        drawState->addCoverageEffect(
                                GrCustomCoordsTextureEffect::Create(fCurrTexture, params),
                                kGlyphCoordsAttributeIndex)->unref();

        if (NULL != fStrike && kARGB_GrMaskFormat == fStrike->getMaskFormat()) {
            drawState->setBlendFunc(fPaint.getSrcBlendCoeff(), fPaint.getDstBlendCoeff());
            drawState->setColor(0xffffffff);
        } else if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_GrBlendCoeff != fPaint.getSrcBlendCoeff() ||
                kISA_GrBlendCoeff != fPaint.getDstBlendCoeff() ||
                fPaint.numColorStages()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            // We don't use the GrPaint's color in this case because it's been premultiplied by
            // alpha. Instead we feed in a non-premultiplied color, and multiply its alpha by
            // the mask texture color. The end result is that we get
            //            mask*paintAlpha*paintColor + (1-mask*paintAlpha)*dstColor
            int a = SkColorGetA(fSkPaint.getColor());
            // paintAlpha
            drawState->setColor(SkColorSetARGB(a, a, a, a));
            // paintColor
            drawState->setBlendConstant(skcolor_to_grcolor_nopremultiply(fSkPaint.getColor()));
            drawState->setBlendFunc(kConstC_GrBlendCoeff, kISC_GrBlendCoeff);
        } else {
            // set back to normal in case we took LCD path previously.
            drawState->setBlendFunc(fPaint.getSrcBlendCoeff(), fPaint.getDstBlendCoeff());
            drawState->setColor(fPaint.getColor());
        }

        int nGlyphs = fCurrVertex / 4;
        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
        fDrawTarget->drawIndexedInstances(kTriangles_GrPrimitiveType,
                                          nGlyphs,
                                          4, 6, &fVertexBounds);

        fDrawTarget->resetVertexSource();
        fVertices = NULL;
        fMaxVertices = 0;
        fCurrVertex = 0;
        fVertexBounds.setLargestInverted();
        SkSafeSetNull(fCurrTexture);
    }
}

inline void GrBitmapTextContext::init(const GrPaint& paint, const SkPaint& skPaint) {
    GrTextContext::init(paint, skPaint);

    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fMaxVertices = 0;
}

inline void GrBitmapTextContext::finish() {
    flushGlyphs();

    GrTextContext::finish();
}

void GrBitmapTextContext::drawText(const GrPaint& paint, const SkPaint& skPaint,
                                   const char text[], size_t byteLength,
                                   SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(paint, skPaint);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &fContext->getMatrix());
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // transform our starting point
    {
        SkPoint loc;
        fContext->getMatrix().mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    // need to measure first
    if (fSkPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector    stop;

        MeasureText(cache, glyphCacheProc, text, byteLength, &stop);

        SkScalar    stopX = stop.fX;
        SkScalar    stopY = stop.fY;

        if (fSkPaint.getTextAlign() == SkPaint::kCenter_Align) {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    }

    const char* stop = text + byteLength;

    SkAutoKern autokern;

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    SkFixed halfSampleX, halfSampleY;
    if (cache->isSubpixel()) {
        halfSampleX = halfSampleY = (SK_FixedHalf >> SkGlyph::kSubBits);
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(fContext->getMatrix());
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_FixedHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_FixedHalf;
        }
    } else {
        halfSampleX = halfSampleY = SK_FixedHalf;
    }

    SkFixed fx = SkScalarToFixed(x) + halfSampleX;
    SkFixed fy = SkScalarToFixed(y) + halfSampleY;

    GrContext::AutoMatrix  autoMatrix;
    autoMatrix.setIdentity(fContext, &fPaint);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

        fx += autokern.adjust(glyph);

        if (glyph.fWidth) {
            this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                          glyph.getSubXFixed(),
                                          glyph.getSubYFixed()),
                                  SkFixedFloorToFixed(fx),
                                  SkFixedFloorToFixed(fy),
                                  fontScaler);
        }

        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    this->finish();
}

///////////////////////////////////////////////////////////////////////////////
// Copied from SkDraw

// last parameter is interpreted as SkFixed [x, y]
// return the fixed position, which may be rounded or not by the caller
//   e.g. subpixel doesn't round
typedef void (*AlignProc)(const SkPoint&, const SkGlyph&, SkIPoint*);

static void leftAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX), SkScalarToFixed(loc.fY));
}

static void centerAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX) - (glyph.fAdvanceX >> 1),
             SkScalarToFixed(loc.fY) - (glyph.fAdvanceY >> 1));
}

static void rightAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX) - glyph.fAdvanceX,
             SkScalarToFixed(loc.fY) - glyph.fAdvanceY);
}

static AlignProc pick_align_proc(SkPaint::Align align) {
    static const AlignProc gProcs[] = {
        leftAlignProc, centerAlignProc, rightAlignProc
    };

    SkASSERT((unsigned)align < SK_ARRAY_COUNT(gProcs));

    return gProcs[align];
}

class BitmapTextMapState {
public:
    mutable SkPoint fLoc;

    BitmapTextMapState(const SkMatrix& matrix, SkScalar y)
        : fMatrix(matrix), fProc(matrix.getMapXYProc()), fY(y) {}

    typedef void (*Proc)(const BitmapTextMapState&, const SkScalar pos[]);

    Proc pickProc(int scalarsPerPosition);

private:
    const SkMatrix&     fMatrix;
    SkMatrix::MapXYProc fProc;
    SkScalar            fY; // ignored by MapXYProc
    // these are only used by Only... procs
    SkScalar            fScaleX, fTransX, fTransformedY;

    static void MapXProc(const BitmapTextMapState& state, const SkScalar pos[]) {
        state.fProc(state.fMatrix, *pos, state.fY, &state.fLoc);
    }

    static void MapXYProc(const BitmapTextMapState& state, const SkScalar pos[]) {
        state.fProc(state.fMatrix, pos[0], pos[1], &state.fLoc);
    }

    static void MapOnlyScaleXProc(const BitmapTextMapState& state,
                                  const SkScalar pos[]) {
        state.fLoc.set(SkScalarMul(state.fScaleX, *pos) + state.fTransX,
                       state.fTransformedY);
    }

    static void MapOnlyTransXProc(const BitmapTextMapState& state,
                                  const SkScalar pos[]) {
        state.fLoc.set(*pos + state.fTransX, state.fTransformedY);
    }
};

BitmapTextMapState::Proc BitmapTextMapState::pickProc(int scalarsPerPosition) {
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    if (1 == scalarsPerPosition) {
        unsigned mtype = fMatrix.getType();
        if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
            return MapXProc;
        } else {
            fScaleX = fMatrix.getScaleX();
            fTransX = fMatrix.getTranslateX();
            fTransformedY = SkScalarMul(fY, fMatrix.getScaleY()) +
                            fMatrix.getTranslateY();
            return (mtype & SkMatrix::kScale_Mask) ?
                        MapOnlyScaleXProc : MapOnlyTransXProc;
        }
    } else {
        return MapXYProc;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrBitmapTextContext::drawPosText(const GrPaint& paint, const SkPaint& skPaint,
                                      const char text[], size_t byteLength,
                                      const SkScalar pos[], SkScalar constY,
                                      int scalarsPerPosition) {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0/* || fRC->isEmpty()*/) {
        return;
    }

    this->init(paint, skPaint);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &fContext->getMatrix());
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // store original matrix before we reset, so we can use it to transform positions
    SkMatrix ctm = fContext->getMatrix();
    GrContext::AutoMatrix  autoMatrix;
    autoMatrix.setIdentity(fContext, &fPaint);

    const char*        stop = text + byteLength;
    AlignProc          alignProc = pick_align_proc(fSkPaint.getTextAlign());
    BitmapTextMapState       tms(ctm, constY);
    BitmapTextMapState::Proc tmsProc = tms.pickProc(scalarsPerPosition);
    SkFixed halfSampleX = 0, halfSampleY = 0;

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(ctm);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
#ifndef SK_IGNORE_SUBPIXEL_AXIS_ALIGN_FIX
            halfSampleY = SK_FixedHalf;
#endif
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
#ifndef SK_IGNORE_SUBPIXEL_AXIS_ALIGN_FIX
            halfSampleX = SK_FixedHalf;
#endif
        }

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                tmsProc(tms, pos);
                SkFixed fx = SkScalarToFixed(tms.fLoc.fX) + halfSampleX;
                SkFixed fy = SkScalarToFixed(tms.fLoc.fY) + halfSampleY;

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                        glyph.getSubXFixed(),
                                                        glyph.getSubYFixed()),
                                          SkFixedFloorToFixed(fx),
                                          SkFixedFloorToFixed(fy),
                                          fontScaler);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                const char* currentText = text;
                const SkGlyph& metricGlyph = glyphCacheProc(cache, &text, 0, 0);

                if (metricGlyph.fWidth) {
                    SkDEBUGCODE(SkFixed prevAdvX = metricGlyph.fAdvanceX;)
                    SkDEBUGCODE(SkFixed prevAdvY = metricGlyph.fAdvanceY;)

                    tmsProc(tms, pos);
                    SkIPoint fixedLoc;
                    alignProc(tms.fLoc, metricGlyph, &fixedLoc);

                    SkFixed fx = fixedLoc.fX + halfSampleX;
                    SkFixed fy = fixedLoc.fY + halfSampleY;

                    // have to call again, now that we've been "aligned"
                    const SkGlyph& glyph = glyphCacheProc(cache, &currentText,
                                                          fx & fxMask, fy & fyMask);
                    // the assumption is that the metrics haven't changed
                    SkASSERT(prevAdvX == glyph.fAdvanceX);
                    SkASSERT(prevAdvY == glyph.fAdvanceY);
                    SkASSERT(glyph.fWidth);

                    this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                        glyph.getSubXFixed(),
                                                        glyph.getSubYFixed()),
                                          SkFixedFloorToFixed(fx),
                                          SkFixedFloorToFixed(fy),
                                          fontScaler);
                }
                pos += scalarsPerPosition;
            }
        }
    } else {    // not subpixel

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    tmsProc(tms, pos);

                    SkFixed fx = SkScalarToFixed(tms.fLoc.fX) + SK_FixedHalf; //halfSampleX;
                    SkFixed fy = SkScalarToFixed(tms.fLoc.fY) + SK_FixedHalf; //halfSampleY;
                    this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                        glyph.getSubXFixed(),
                                                        glyph.getSubYFixed()),
                                          SkFixedFloorToFixed(fx),
                                          SkFixedFloorToFixed(fy),
                                          fontScaler);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    tmsProc(tms, pos);

                    SkIPoint fixedLoc;
                    alignProc(tms.fLoc, glyph, &fixedLoc);

                    SkFixed fx = fixedLoc.fX + SK_FixedHalf; //halfSampleX;
                    SkFixed fy = fixedLoc.fY + SK_FixedHalf; //halfSampleY;
                    this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                        glyph.getSubXFixed(),
                                                        glyph.getSubYFixed()),
                                          SkFixedFloorToFixed(fx),
                                          SkFixedFloorToFixed(fy),
                                          fontScaler);
                }
                pos += scalarsPerPosition;
            }
        }
    }

    this->finish();
}

namespace {

// position + texture coord
extern const GrVertexAttrib gTextVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

};

void GrBitmapTextContext::drawPackedGlyph(GrGlyph::PackedID packed,
                                          GrFixed vx, GrFixed vy,
                                          GrFontScaler* scaler) {
    if (NULL == fDrawTarget) {
        return;
    }

    if (NULL == fStrike) {
        fStrike = fContext->getFontCache()->getStrike(scaler, false);
    }

    GrGlyph* glyph = fStrike->getGlyph(packed, scaler);
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return;
    }

    vx += SkIntToFixed(glyph->fBounds.fLeft);
    vy += SkIntToFixed(glyph->fBounds.fTop);

    // keep them as ints until we've done the clip-test
    GrFixed width = glyph->fBounds.width();
    GrFixed height = glyph->fBounds.height();

    // check if we clipped out
    if (true || NULL == glyph->fPlot) {
        int x = vx >> 16;
        int y = vy >> 16;
        if (fClipRect.quickReject(x, y, x + width, y + height)) {
//            SkCLZ(3);    // so we can set a break-point in the debugger
            return;
        }
    }

    if (NULL == glyph->fPlot) {
        if (fStrike->addGlyphToAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        // try to clear out an unused plot before we flush
        if (fContext->getFontCache()->freeUnusedPlot(fStrike) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        if (c_DumpFontCache) {
#ifdef SK_DEVELOPER
            fContext->getFontCache()->dump();
#endif
        }

        // flush any accumulated draws to allow us to free up a plot
        this->flushGlyphs();
        fContext->flush();

        // we should have an unused plot now
        if (fContext->getFontCache()->freeUnusedPlot(fStrike) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        if (NULL == glyph->fPath) {
            SkPath* path = SkNEW(SkPath);
            if (!scaler->getGlyphPath(glyph->glyphID(), path)) {
                // flag the glyph as being dead?
                delete path;
                return;
            }
            glyph->fPath = path;
        }

        GrContext::AutoMatrix am;
        SkMatrix translate;
        translate.setTranslate(SkFixedToScalar(vx - SkIntToFixed(glyph->fBounds.fLeft)),
                               SkFixedToScalar(vy - SkIntToFixed(glyph->fBounds.fTop)));
        GrPaint tmpPaint(fPaint);
        am.setPreConcat(fContext, translate, &tmpPaint);
        SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(tmpPaint, *glyph->fPath, stroke);
        return;
    }

HAS_ATLAS:
    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

    // now promote them to fixed (TODO: Rethink using fixed pt).
    width = SkIntToFixed(width);
    height = SkIntToFixed(height);

    GrTexture* texture = glyph->fPlot->texture();
    SkASSERT(texture);

    if (fCurrTexture != texture || fCurrVertex + 4 > fMaxVertices) {
        this->flushGlyphs();
        fCurrTexture = texture;
        fCurrTexture->ref();
    }

    if (NULL == fVertices) {
       // If we need to reserve vertices allow the draw target to suggest
        // a number of verts to reserve and whether to perform a flush.
        fMaxVertices = kMinRequestedVerts;
        fDrawTarget->drawState()->setVertexAttribs<gTextVertexAttribs>(
            SK_ARRAY_COUNT(gTextVertexAttribs));
        bool flush = fDrawTarget->geometryHints(&fMaxVertices, NULL);
        if (flush) {
            this->flushGlyphs();
            fContext->flush();
            fDrawTarget->drawState()->setVertexAttribs<gTextVertexAttribs>(
                SK_ARRAY_COUNT(gTextVertexAttribs));
        }
        fMaxVertices = kDefaultRequestedVerts;
        // ignore return, no point in flushing again.
        fDrawTarget->geometryHints(&fMaxVertices, NULL);

        int maxQuadVertices = 4 * fContext->getQuadIndexBuffer()->maxQuads();
        if (fMaxVertices < kMinRequestedVerts) {
            fMaxVertices = kDefaultRequestedVerts;
        } else if (fMaxVertices > maxQuadVertices) {
            // don't exceed the limit of the index buffer
            fMaxVertices = maxQuadVertices;
        }
        bool success = fDrawTarget->reserveVertexAndIndexSpace(fMaxVertices,
                                                               0,
                                                               GrTCast<void**>(&fVertices),
                                                               NULL);
        GrAlwaysAssert(success);
        SkASSERT(2*sizeof(GrPoint) == fDrawTarget->getDrawState().getVertexSize());
    }

    GrFixed tx = SkIntToFixed(glyph->fAtlasLocation.fX);
    GrFixed ty = SkIntToFixed(glyph->fAtlasLocation.fY);

    SkRect r;
    r.fLeft = SkFixedToFloat(vx);
    r.fTop = SkFixedToFloat(vy);
    r.fRight = SkFixedToFloat(vx + width);
    r.fBottom = SkFixedToFloat(vy + height);

    fVertexBounds.growToInclude(r);

    fVertices[2*fCurrVertex].setRectFan(r.fLeft, r.fTop, r.fRight, r.fBottom,
                                        2 * sizeof(SkPoint));
    fVertices[2*fCurrVertex+1].setRectFan(SkFixedToFloat(texture->normalizeFixedX(tx)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty)),
                                          SkFixedToFloat(texture->normalizeFixedX(tx + width)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty + height)),
                                          2 * sizeof(SkPoint));
    fCurrVertex += 4;
}
