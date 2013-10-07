/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrTextContext.h"
#include "GrAtlas.h"
#include "GrContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrIndexBuffer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "effects/GrCustomCoordsTextureEffect.h"

static const int kGlyphCoordsAttributeIndex = 1;

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

void GrTextContext::flushGlyphs() {
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

        if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_GrBlendCoeff != fPaint.getSrcBlendCoeff() ||
                kISA_GrBlendCoeff != fPaint.getDstBlendCoeff() ||
                fPaint.numColorStages()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            // setup blend so that we get mask * paintColor + (1-mask)*dstColor
            drawState->setBlendConstant(fPaint.getColor());
            drawState->setBlendFunc(kConstC_GrBlendCoeff, kISC_GrBlendCoeff);
            // don't modulate by the paint's color in the frag since we're
            // already doing it via the blend const.
            drawState->setColor(0xffffffff);
        } else {
            // set back to normal in case we took LCD path previously.
            drawState->setBlendFunc(fPaint.getSrcBlendCoeff(), fPaint.getDstBlendCoeff());
            drawState->setColor(fPaint.getColor());
        }

        int nGlyphs = fCurrVertex / 4;
        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
        fDrawTarget->drawIndexedInstances(kTriangles_GrPrimitiveType,
                                          nGlyphs,
                                          4, 6);
        fDrawTarget->resetVertexSource();
        fVertices = NULL;
        fMaxVertices = 0;
        fCurrVertex = 0;
        SkSafeSetNull(fCurrTexture);
    }
}

GrTextContext::GrTextContext(GrContext* context, const GrPaint& paint) : fPaint(paint) {
    fContext = context;
    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    const GrClipData* clipData = context->getClip();

    SkRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     context->getRenderTarget()->width(),
                                     context->getRenderTarget()->height(),
                                     &devConservativeBound);

    devConservativeBound.roundOut(&fClipRect);

    fAutoMatrix.setIdentity(fContext, &fPaint);

    fDrawTarget = fContext->getTextTarget();

    fVertices = NULL;
    fMaxVertices = 0;
}

GrTextContext::~GrTextContext() {
    this->flushGlyphs();
}

void GrTextContext::flush() {
    this->flushGlyphs();
}

namespace {

// position + texture coord
extern const GrVertexAttrib gTextVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

};

void GrTextContext::drawPackedGlyph(GrGlyph::PackedID packed,
                                    GrFixed vx, GrFixed vy,
                                    GrFontScaler* scaler) {
    if (NULL == fDrawTarget) {
        return;
    }
    if (NULL == fStrike) {
        fStrike = fContext->getFontCache()->getStrike(scaler);
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
        if (fStrike->getGlyphAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        // try to clear out an unused plot before we flush
        fContext->getFontCache()->freePlotExceptFor(fStrike);
        if (fStrike->getGlyphAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        if (c_DumpFontCache) {
#ifdef SK_DEVELOPER
            fContext->getFontCache()->dump();
#endif
        }

        // before we purge the cache, we must flush any accumulated draws
        this->flushGlyphs();
        fContext->flush();

        // try to purge
        fContext->getFontCache()->purgeExceptFor(fStrike);
        // need to use new flush count here
        if (fStrike->getGlyphAtlas(glyph, scaler)) {
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

    fVertices[2*fCurrVertex].setRectFan(SkFixedToFloat(vx),
                                        SkFixedToFloat(vy),
                                        SkFixedToFloat(vx + width),
                                        SkFixedToFloat(vy + height),
                                        2 * sizeof(SkPoint));
    fVertices[2*fCurrVertex+1].setRectFan(SkFixedToFloat(texture->normalizeFixedX(tx)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty)),
                                          SkFixedToFloat(texture->normalizeFixedX(tx + width)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty + height)),
                                          2 * sizeof(SkPoint));
    fCurrVertex += 4;
}
