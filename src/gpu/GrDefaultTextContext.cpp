
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrAtlas.h"
#include "GrDefaultTextContext.h"
#include "GrContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrGpuVertex.h"
#include "GrTemplates.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"

void GrDefaultTextContext::flushGlyphs() {
    GrAssert(this->isValid());
    if (fCurrVertex > 0) {
        GrDrawState* drawState = fDrawTarget->drawState();
        // setup our sampler state for our text texture/atlas
        GrSamplerState::Filter filter;
        if (fExtMatrix.isIdentity()) {
            filter = GrSamplerState::kNearest_Filter;
        } else {
            filter = GrSamplerState::kBilinear_Filter;
        }
        drawState->sampler(kGlyphMaskStage)->reset(
            GrSamplerState::kRepeat_WrapMode,filter);

        GrAssert(GrIsALIGN4(fCurrVertex));
        GrAssert(fCurrTexture);
        drawState->setTexture(kGlyphMaskStage, fCurrTexture);

        if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_BlendCoeff != fGrPaint.fSrcBlendCoeff ||
                kISA_BlendCoeff != fGrPaint.fDstBlendCoeff ||
                fGrPaint.hasTexture()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            // setup blend so that we get mask * paintColor + (1-mask)*dstColor
            drawState->setBlendConstant(fGrPaint.fColor);
            drawState->setBlendFunc(kConstC_BlendCoeff, kISC_BlendCoeff);
            // don't modulate by the paint's color in the frag since we're
            // already doing it via the blend const.
            drawState->setColor(0xffffffff);
        } else {
            // set back to normal in case we took LCD path previously.
            drawState->setBlendFunc(fGrPaint.fSrcBlendCoeff, fGrPaint.fDstBlendCoeff);
            drawState->setColor(fGrPaint.fColor);
        }

        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
        int nGlyphs = fCurrVertex / 4;
        fDrawTarget->drawIndexedInstances(kTriangles_PrimitiveType,
                                          nGlyphs,
                                          4, 6);
        fVertices = NULL;
        this->INHERITED::reset();
    }
}

GrDefaultTextContext::GrDefaultTextContext() {
}

GrDefaultTextContext::~GrDefaultTextContext() {
}

void GrDefaultTextContext::init(GrContext* context,
                                const GrPaint& paint,
                                const GrMatrix* extMatrix) {
    this->INHERITED::init(context, paint, extMatrix);

    fStrike = NULL;

    if (NULL != extMatrix) {
        fExtMatrix = *extMatrix;
    } else {
        fExtMatrix.reset();
    }
    if (context->getClip().hasConservativeBounds()) {
        if (!fExtMatrix.isIdentity()) {
            GrMatrix inverse;
            GrRect r = context->getClip().getConservativeBounds();
            if (fExtMatrix.invert(&inverse)) {
                inverse.mapRect(&r);
                r.roundOut(&fClipRect);
            }
        } else {
            context->getClip().getConservativeBounds().roundOut(&fClipRect);
        }
    } else {
        fClipRect.setLargest();
    }

    // save the context's original matrix off and restore in destructor
    // getTextTarget should be called after that
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
        if (NULL != fGrPaint.getTexture(t)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fGrPaint.textureSampler(t)->preConcatMatrix(invVM);
            }
        }
    }
    for (int m = 0; m < GrPaint::kMaxMasks; ++m) {
        if (NULL != fGrPaint.getMask(m)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fGrPaint.maskSampler(m)->preConcatMatrix(invVM);
            }
        }
    }

    // this has been already done in the baseclass, but we need to repeat
    // due to new matrix
    fDrawTarget = fContext->getTextTarget(fGrPaint);

    fVertices = NULL;

    fVertexLayout = 
        GrDrawTarget::kTextFormat_VertexLayoutBit |
        GrDrawTarget::StageTexCoordVertexLayoutBit(kGlyphMaskStage, 0);

    int stageMask = paint.getActiveStageMask();
    if (stageMask) {
        for (int i = 0; i < GrPaint::kTotalStages; ++i) {
            if ((1 << i) & stageMask) {
                fVertexLayout |= 
                    GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(i);
                GrAssert(i != kGlyphMaskStage);
            }
        }
    }
}

void GrDefaultTextContext::finish() {
    this->flush();

    fStrike = NULL;
    fContext->setMatrix(fOrigViewMatrix);

    this->INHERITED::finish();
}

void GrDefaultTextContext::flush() {
    GrAssert(this->isValid());
    this->flushGlyphs();
}

static inline void setRectFan(GrGpuTextVertex v[4], int l, int t, int r, int b,
                              int stride) {
    v[0 * stride].setI(l, t);
    v[1 * stride].setI(l, b);
    v[2 * stride].setI(r, b);
    v[3 * stride].setI(r, t);
}

void GrDefaultTextContext::drawPackedGlyph(GrGlyph::PackedID packed,
                                    GrFixed vx, GrFixed vy,
                                    GrFontScaler* scaler) {
    GrAssert(this->isValid());
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
            SkPath* path = new SkPath;
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
        fContext->drawPath(fGrPaint, *glyph->fPath, kWinding_PathFill,
                           &translate);
        return;
    }

HAS_ATLAS:
    GrAssert(glyph->fAtlas);

    // now promote them to fixed
    width = GrIntToFixed(width);
    height = GrIntToFixed(height);

    GrTexture* texture = glyph->fAtlas->texture();
    this->prepareForGlyph(texture);

    this->setupVertexBuff(GrTCast<void**>(&fVertices),
                          fVertexLayout);

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
