/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrTextContext.h"
#include "GrAtlas.h"
#include "GrContext.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "GrFontScaler.h"
#include "GrIndexBuffer.h"
#include "GrGpuVertex.h"
#include "GrDrawTarget.h"

enum {
    kGlyphMaskStage = GrPaint::kTotalStages,
};

void GrTextContext::flushGlyphs() {
    if (fCurrVertex > 0) {
        GrDrawTarget::AutoStateRestore asr(fDrawTarget);

        // setup our sampler state for our text texture/atlas
        GrSamplerState::Filter filter;
        if (fExtMatrix.isIdentity()) {
            filter = GrSamplerState::kNearest_Filter;
        } else {
            filter = GrSamplerState::kBilinear_Filter;
        }
        GrSamplerState sampler(GrSamplerState::kRepeat_WrapMode,
                               GrSamplerState::kRepeat_WrapMode,
                               filter);
        fDrawTarget->setSamplerState(kGlyphMaskStage, sampler);

        GrAssert(GrIsALIGN4(fCurrVertex));
        int nIndices = fCurrVertex + (fCurrVertex >> 1);
        GrAssert(fCurrTexture);
        fDrawTarget->setTexture(kGlyphMaskStage, fCurrTexture);

        if (!GrPixelConfigIsAlphaOnly(fCurrTexture->config())) {
            if (kOne_BlendCoeff != fPaint.fSrcBlendCoeff ||
                kISA_BlendCoeff != fPaint.fDstBlendCoeff ||
                fPaint.hasTexture()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            // setup blend so that we get mask * paintColor + (1-mask)*dstColor
            fDrawTarget->setBlendConstant(fPaint.fColor);
            fDrawTarget->setBlendFunc(kConstC_BlendCoeff, kISC_BlendCoeff);
            // don't modulate by the paint's color in the frag since we're
            // already doing it via the blend const.
            fDrawTarget->setColor(0xffffffff);
        } else {
            // set back to normal in case we took LCD path previously.
            fDrawTarget->setBlendFunc(fPaint.fSrcBlendCoeff, fPaint.fDstBlendCoeff);
            fDrawTarget->setColor(fPaint.fColor);
        }

        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());

        fDrawTarget->drawIndexed(kTriangles_PrimitiveType,
                                 0, 0, fCurrVertex, nIndices);
        fDrawTarget->resetVertexSource();
        fVertices = NULL;
        fMaxVertices = 0;
        fCurrVertex = 0;
        fCurrTexture->unref();
        fCurrTexture = NULL;
    }
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
        fExtMatrix = GrMatrix::I();
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
        if (NULL != fPaint.getTexture(t)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fPaint.getTextureSampler(t)->preConcatMatrix(invVM);
            }
        }
    }
    for (int m = 0; m < GrPaint::kMaxMasks; ++m) {
        if (NULL != fPaint.getMask(m)) {
            if (invVMComputed || fOrigViewMatrix.invert(&invVM)) {
                invVMComputed = true;
                fPaint.getMaskSampler(m)->preConcatMatrix(invVM);
            }
        }
    }

    fDrawTarget = fContext->getTextTarget(fPaint);

    fVertices = NULL;
    fMaxVertices = 0;

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

GrTextContext::~GrTextContext() {
    this->flushGlyphs();
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
        fContext->flushText();

        // try to purge
        fContext->getFontCache()->purgeExceptFor(fStrike);
        if (fStrike->getGlyphAtlas(glyph, scaler)) {
            goto HAS_ATLAS;
        }

        if (NULL == glyph->fPath) {
            GrPath* path = new GrPath;
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
        fContext->drawPath(fPaint, *glyph->fPath, kWinding_PathFill,
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
        bool flush = fDrawTarget->geometryHints(fVertexLayout,
                                               &fMaxVertices,
                                               NULL);
        if (flush) {
            this->flushGlyphs();
            fContext->flushText();
            fDrawTarget = fContext->getTextTarget(fPaint);
            fMaxVertices = kDefaultRequestedVerts;
            // ignore return, no point in flushing again.
            fDrawTarget->geometryHints(fVertexLayout,
                                       &fMaxVertices,
                                       NULL);
        }

        int maxQuadVertices = 4 * fContext->getQuadIndexBuffer()->size() / (6 * sizeof(uint16_t));
        if (fMaxVertices < kMinRequestedVerts) {
            fMaxVertices = kDefaultRequestedVerts;
        } else if (fMaxVertices > maxQuadVertices) {
            // don't exceed the limit of the index buffer
            fMaxVertices = maxQuadVertices;
        }
        bool success = fDrawTarget->reserveVertexSpace(fVertexLayout, 
                                                   fMaxVertices,
                                                   GrTCast<void**>(&fVertices));
        GrAlwaysAssert(success);
    }

    GrFixed tx = GrIntToFixed(glyph->fAtlasLocation.fX);
    GrFixed ty = GrIntToFixed(glyph->fAtlasLocation.fY);

#if GR_GL_TEXT_TEXTURE_NORMALIZED
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


