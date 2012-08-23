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
        drawState->sampler(kGlyphMaskStage)->reset(SkShader::kRepeat_TileMode,
                                                   !fExtMatrix.isIdentity());

        GrAssert(GrIsALIGN4(fCurrVertex));
        GrAssert(fCurrTexture);
        drawState->createTextureEffect(kGlyphMaskStage, fCurrTexture);

        if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_GrBlendCoeff != fPaint.fSrcBlendCoeff ||
                kISA_GrBlendCoeff != fPaint.fDstBlendCoeff ||
                fPaint.hasTexture()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            // setup blend so that we get mask * paintColor + (1-mask)*dstColor
            drawState->setBlendConstant(fPaint.fColor);
            drawState->setBlendFunc(kConstC_GrBlendCoeff, kISC_GrBlendCoeff);
            // don't modulate by the paint's color in the frag since we're
            // already doing it via the blend const.
            drawState->setColor(0xffffffff);
        } else {
            // set back to normal in case we took LCD path previously.
            drawState->setBlendFunc(fPaint.fSrcBlendCoeff,
                                    fPaint.fDstBlendCoeff);
            drawState->setColor(fPaint.fColor);
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

GrTextContext::GrTextContext(GrContext* context,
                             const GrPaint& paint,
                             const GrMatrix* extMatrix) : fPaint(paint) {
    fContext = context;
    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    if (NULL != extMatrix) {
        fExtMatrix = *extMatrix;
    } else {
        fExtMatrix.reset();
    }

    const GrClipData* clipData = context->getClip();

    GrRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     context->getRenderTarget()->width(),
                                     context->getRenderTarget()->height(),
                                     &devConservativeBound);

    if (!fExtMatrix.isIdentity()) {
        GrMatrix inverse;
        if (fExtMatrix.invert(&inverse)) {
            inverse.mapRect(&devConservativeBound);
        }
    }

    devConservativeBound.roundOut(&fClipRect);

    // save the context's original matrix off and restore in destructor
    // this must be done before getTextTarget.
    fOrigViewMatrix = fContext->getMatrix();
    fContext->setMatrix(fExtMatrix);

    /*
     We need to call preConcatMatrix with our viewmatrix's inverse, for each
     texture and mask in the paint. However, computing the inverse can be
     expensive, and its possible we may not have any textures or masks, so these
     two loops are written such that we only compute the inverse (once) if we
     need it. We do this on our copy of the paint rather than directly on the
     draw target because we re-provide the paint to the context when we have
     to flush our glyphs or draw a glyph as a path midstream.
    */
    bool invVMComputed = false;
    GrMatrix invVM;
    for (int t = 0; t < GrPaint::kMaxTextures; ++t) {
        if (fPaint.isTextureStageEnabled(t)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fPaint.textureSampler(t)->preConcatMatrix(invVM);
            }
        }
    }
    for (int m = 0; m < GrPaint::kMaxMasks; ++m) {
        if (fPaint.isMaskStageEnabled(m)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fPaint.maskSampler(m)->preConcatMatrix(invVM);
            }
        }
    }

    fDrawTarget = NULL;

    fVertices = NULL;
    fMaxVertices = 0;

    fVertexLayout =
        GrDrawTarget::kTextFormat_VertexLayoutBit |
        GrDrawTarget::StageTexCoordVertexLayoutBit(kGlyphMaskStage, 0);
}

GrTextContext::~GrTextContext() {
    this->flushGlyphs();
    if (fDrawTarget) {
        fDrawTarget->drawState()->disableStages();
    }
    fContext->setMatrix(fOrigViewMatrix);
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

    vx += GrIntToFixed(glyph->fBounds.fLeft);
    vy += GrIntToFixed(glyph->fBounds.fTop);

    // keep them as ints until we've done the clip-test
    GrFixed width = glyph->fBounds.width();
    GrFixed height = glyph->fBounds.height();

    // check if we clipped out
    if (true || NULL == glyph->fAtlas) {
        int x = vx >> 16;
        int y = vy >> 16;
        if (fClipRect.quickReject(x, y, x + width, y + height)) {
//            Gr_clz(3);    // so we can set a break-point in the debugger
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

        GrPoint translate;
        translate.set(GrFixedToScalar(vx - GrIntToFixed(glyph->fBounds.fLeft)),
                      GrFixedToScalar(vy - GrIntToFixed(glyph->fBounds.fTop)));
        fContext->drawPath(fPaint, *glyph->fPath, kWinding_GrPathFill,
                           &translate);
        return;
    }

HAS_ATLAS:
    GrAssert(glyph->fAtlas);

    // now promote them to fixed
    width = GrIntToFixed(width);
    height = GrIntToFixed(height);

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
                     fDrawTarget->geometryHints(fVertexLayout,
                                                &fMaxVertices,
                                                NULL);
        if (flush) {
            this->flushGlyphs();
            fContext->flush();
        }
        fDrawTarget = fContext->getTextTarget(fPaint);
        fMaxVertices = kDefaultRequestedVerts;
        // ignore return, no point in flushing again.
        fDrawTarget->geometryHints(fVertexLayout,
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

    GrFixed tx = GrIntToFixed(glyph->fAtlasLocation.fX);
    GrFixed ty = GrIntToFixed(glyph->fAtlasLocation.fY);

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

