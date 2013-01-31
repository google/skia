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
#include "GrGpuVertex.h"
#include "GrIndexBuffer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkPath.h"
#include "SkStrokeRec.h"

enum {
    kGlyphMaskStage = GrPaint::kTotalStages,
};

void GrTextContext::flushGlyphs() {
    if (NULL == fDrawTarget) {
        return;
    }
    GrDrawState* drawState = fDrawTarget->drawState();
    if (fCurrVertex > 0) {
        // setup our sampler state for our text texture/atlas
        GrAssert(GrIsALIGN4(fCurrVertex));
        GrAssert(fCurrTexture);
        GrTextureParams params(SkShader::kRepeat_TileMode, false);
        drawState->createTextureEffect(kGlyphMaskStage, fCurrTexture, SkMatrix::I(), params);

        if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_GrBlendCoeff != fPaint.getSrcBlendCoeff() ||
                kISA_GrBlendCoeff != fPaint.getDstBlendCoeff() ||
                fPaint.hasColorStage()) {
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
        GrSafeSetNull(fCurrTexture);
    }
    drawState->disableStages();
    fDrawTarget = NULL;
}

GrTextContext::GrTextContext(GrContext* context, const GrPaint& paint) : fPaint(paint) {
    fContext = context;
    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    const GrClipData* clipData = context->getClip();

    GrRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     context->getRenderTarget()->width(),
                                     context->getRenderTarget()->height(),
                                     &devConservativeBound);

    devConservativeBound.roundOut(&fClipRect);

    fAutoMatrix.setIdentity(fContext, &fPaint);

    fDrawTarget = NULL;

    fVertices = NULL;
    fMaxVertices = 0;

    fVertexLayout =
        GrDrawState::kTextFormat_VertexLayoutBit |
        GrDrawState::StageTexCoordVertexLayoutBit(kGlyphMaskStage, 0);
}

GrTextContext::~GrTextContext() {
    this->flushGlyphs();
    if (fDrawTarget) {
        fDrawTarget->drawState()->disableStages();
    }
}

void GrTextContext::flush() {
    this->flushGlyphs();
}

static inline void setRectFan(GrGpuTextVertex v[4], int l, int t, int r, int b,
                              int stride) {
    v[0 * stride].setI(l, t);
    v[1 * stride].setI(l, b);
    v[2 * stride].setI(r, b);
    v[3 * stride].setI(r, t);
}

void GrTextContext::drawPackedGlyph(GrGlyph::PackedID packed,
                                    GrFixed vx, GrFixed vy,
                                    GrFontScaler* scaler) {
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
    if (true || NULL == glyph->fAtlas) {
        int x = vx >> 16;
        int y = vy >> 16;
        if (fClipRect.quickReject(x, y, x + width, y + height)) {
//            SkCLZ(3);    // so we can set a break-point in the debugger
            return;
        }
    }

    if (NULL == glyph->fAtlas) {
        if (fStrike->getGlyphAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        // before we purge the cache, we must flush any accumulated draws
        this->flushGlyphs();
        fContext->flush();

        // try to purge
        fContext->getFontCache()->purgeExceptFor(fStrike);
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
    GrAssert(glyph->fAtlas);

    // now promote them to fixed
    width = SkIntToFixed(width);
    height = SkIntToFixed(height);

    GrTexture* texture = glyph->fAtlas->texture();
    GrAssert(texture);

    if (fCurrTexture != texture || fCurrVertex + 4 > fMaxVertices) {
        this->flushGlyphs();
        fCurrTexture = texture;
        fCurrTexture->ref();
    }

    if (NULL == fVertices) {
        // If we need to reserve vertices allow the draw target to suggest
        // a number of verts to reserve and whether to perform a flush.
        fMaxVertices = kMinRequestedVerts;
        bool flush = (NULL != fDrawTarget) &&
                     fDrawTarget->geometryHints(GrDrawState::VertexSize(fVertexLayout),
                                                &fMaxVertices,
                                                NULL);
        if (flush) {
            this->flushGlyphs();
            fContext->flush();
        }
        fDrawTarget = fContext->getTextTarget(fPaint);
        fMaxVertices = kDefaultRequestedVerts;
        // ignore return, no point in flushing again.
        fDrawTarget->geometryHints(GrDrawState::VertexSize(fVertexLayout),
                                   &fMaxVertices,
                                   NULL);

        int maxQuadVertices = 4 * fContext->getQuadIndexBuffer()->maxQuads();
        if (fMaxVertices < kMinRequestedVerts) {
            fMaxVertices = kDefaultRequestedVerts;
        } else if (fMaxVertices > maxQuadVertices) {
            // don't exceed the limit of the index buffer
            fMaxVertices = maxQuadVertices;
        }
        bool success = fDrawTarget->reserveVertexAndIndexSpace(
                                                   fVertexLayout,
                                                   fMaxVertices,
                                                   0,
                                                   GrTCast<void**>(&fVertices),
                                                   NULL);
        GrAlwaysAssert(success);
    }

    GrFixed tx = SkIntToFixed(glyph->fAtlasLocation.fX);
    GrFixed ty = SkIntToFixed(glyph->fAtlasLocation.fY);

#if GR_TEXT_SCALAR_IS_USHORT
    int x = vx >> 16;
    int y = vy >> 16;
    int w = width >> 16;
    int h = height >> 16;

    setRectFan(&fVertices[2*fCurrVertex], x, y, x + w, y + h, 2);
    setRectFan(&fVertices[2*fCurrVertex+1],
               texture->normalizeFixedX(tx),
               texture->normalizeFixedY(ty),
               texture->normalizeFixedX(tx + width),
               texture->normalizeFixedY(ty + height),
               2);
#else
    fVertices[2*fCurrVertex].setXRectFan(vx, vy, vx + width, vy + height,
                                        2 * sizeof(GrGpuTextVertex));
    fVertices[2*fCurrVertex+1].setXRectFan(texture->normalizeFixedX(tx),
                                          texture->normalizeFixedY(ty),
                                          texture->normalizeFixedX(tx + width),
                                          texture->normalizeFixedY(ty + height),
                                          2 * sizeof(GrGpuTextVertex));
#endif
    fCurrVertex += 4;
}
