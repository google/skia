/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextContext.h"
#include "GrAtlas.h"
#include "SkColorFilter.h"
#include "GrDrawTarget.h"
#include "GrDrawTargetCaps.h"
#include "GrFontScaler.h"
#include "SkGlyphCache.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrStrokeInfo.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "effects/GrDistanceFieldTextureEffect.h"

static const int kGlyphCoordsAttributeIndex = 1;

static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 64;
static const int kMediumDFFontLimit = 64;
static const int kLargeDFFontSize = 128;

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

GrDistanceFieldTextContext::GrDistanceFieldTextContext(GrContext* context,
                                                       const SkDeviceProperties& properties,
                                                       bool enable)
                                                    : GrTextContext(context, properties) {
#if SK_FORCE_DISTANCEFIELD_FONTS
    fEnableDFRendering = true;
#else
    fEnableDFRendering = enable;
#endif
    fStrike = NULL;
    fGammaTexture = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fMaxVertices = 0;
}

GrDistanceFieldTextContext::~GrDistanceFieldTextContext() {
    this->flushGlyphs();
    SkSafeSetNull(fGammaTexture);
}

bool GrDistanceFieldTextContext::canDraw(const SkPaint& paint) {
    if (!fEnableDFRendering && !paint.isDistanceFieldTextTEMP()) {
        return false;
    }

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (paint.getRasterizer() || paint.getMaskFilter() ||
        !fContext->getTextTarget()->caps()->shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (paint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    // TODO: choose an appropriate maximum scale for distance fields and
    //       enable perspective
    if (SkDraw::ShouldDrawTextAsPaths(paint, fContext->getMatrix())) {
        return false;
    }

    // distance fields cannot represent color fonts
    SkScalerContext::Rec    rec;
    SkScalerContext::MakeRec(paint, &fDeviceProperties, NULL, &rec);
    return rec.getFormat() != SkMask::kARGB32_Format;
}

static inline GrColor skcolor_to_grcolor_nopremultiply(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    return GrColorPackRGBA(r, g, b, 0xff);
}

void GrDistanceFieldTextContext::flushGlyphs() {
    if (NULL == fDrawTarget) {
        return;
    }

    GrDrawState* drawState = fDrawTarget->drawState();
    GrDrawState::AutoRestoreEffects are(drawState);
    drawState->setFromPaint(fPaint, fContext->getMatrix(), fContext->getRenderTarget());

    if (fCurrVertex > 0) {
        // setup our sampler state for our text texture/atlas
        SkASSERT(SkIsAlign4(fCurrVertex));
        SkASSERT(fCurrTexture);
        GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kBilerp_FilterMode);
        GrTextureParams gammaParams(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);

        // Effects could be stored with one of the cache objects (atlas?)
        SkColor filteredColor;
        SkColorFilter* colorFilter = fSkPaint.getColorFilter();
        if (NULL != colorFilter) {
            filteredColor = colorFilter->filterColor(fSkPaint.getColor());
        } else {
            filteredColor = fSkPaint.getColor();
        }
        if (fUseLCDText) {
            GrColor colorNoPreMul = skcolor_to_grcolor_nopremultiply(filteredColor);
            bool useBGR = SkDeviceProperties::Geometry::kBGR_Layout ==
                                                            fDeviceProperties.fGeometry.getLayout();
            drawState->addCoverageEffect(GrDistanceFieldLCDTextureEffect::Create(
                                                            fCurrTexture,
                                                            params,
                                                            fGammaTexture,
                                                            gammaParams,
                                                            colorNoPreMul,
                                                            fContext->getMatrix().rectStaysRect() &&
                                                            fContext->getMatrix().isSimilarity(),
                                                            useBGR),
                                         kGlyphCoordsAttributeIndex)->unref();

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
            drawState->setBlendConstant(colorNoPreMul);
            drawState->setBlendFunc(kConstC_GrBlendCoeff, kISC_GrBlendCoeff);
        } else {
#ifdef SK_GAMMA_APPLY_TO_A8
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(fDeviceProperties.fGamma,
                                                                filteredColor);
            drawState->addCoverageEffect(GrDistanceFieldTextureEffect::Create(
                                                              fCurrTexture, params,
                                                              fGammaTexture, gammaParams,
                                                              lum/255.f,
                                                              fContext->getMatrix().isSimilarity()),
                                         kGlyphCoordsAttributeIndex)->unref();
#else
            drawState->addCoverageEffect(GrDistanceFieldTextureEffect::Create(
                                                              fCurrTexture, params,
                                                              fContext->getMatrix().isSimilarity()),
                                         kGlyphCoordsAttributeIndex)->unref();
#endif
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

namespace {

// position + texture coord
extern const GrVertexAttrib gTextVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(SkPoint), kEffect_GrVertexAttribBinding}
};

};

void GrDistanceFieldTextContext::drawPackedGlyph(GrGlyph::PackedID packed,
                                                 SkFixed vx, SkFixed vy,
                                                 GrFontScaler* scaler) {
    if (NULL == fDrawTarget) {
        return;
    }
    if (NULL == fStrike) {
        fStrike = fContext->getFontCache()->getStrike(scaler, true);
    }

    GrGlyph* glyph = fStrike->getGlyph(packed, scaler);
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return;
    }

    SkScalar sx = SkFixedToScalar(vx);
    SkScalar sy = SkFixedToScalar(vy);
/*
    // not valid, need to find a different solution for this
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
*/
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

        // before we purge the cache, we must flush any accumulated draws
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
        SkMatrix ctm;
        ctm.setScale(fTextRatio, fTextRatio);
        ctm.postTranslate(sx, sy);
        GrPaint tmpPaint(fPaint);
        am.setPreConcat(fContext, ctm, &tmpPaint);
        GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(tmpPaint, *glyph->fPath, strokeInfo);
        return;
    }

HAS_ATLAS:
    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

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
        SkASSERT(2*sizeof(SkPoint) == fDrawTarget->getDrawState().getVertexSize());
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2*SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2*SK_DistanceFieldInset);

    SkScalar scale = fTextRatio;
    dx *= scale;
    dy *= scale;
    sx += dx;
    sy += dy;
    width *= scale;
    height *= scale;

    SkFixed tx = SkIntToFixed(glyph->fAtlasLocation.fX + SK_DistanceFieldInset);
    SkFixed ty = SkIntToFixed(glyph->fAtlasLocation.fY + SK_DistanceFieldInset);
    SkFixed tw = SkIntToFixed(glyph->fBounds.width() - 2*SK_DistanceFieldInset);
    SkFixed th = SkIntToFixed(glyph->fBounds.height() - 2*SK_DistanceFieldInset);

    static const size_t kVertexSize = 2 * sizeof(SkPoint);
    fVertices[2*fCurrVertex].setRectFan(sx,
                                        sy,
                                        sx + width,
                                        sy + height,
                                        kVertexSize);
    fVertices[2*fCurrVertex+1].setRectFan(SkFixedToFloat(texture->normalizeFixedX(tx)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty)),
                                          SkFixedToFloat(texture->normalizeFixedX(tx + tw)),
                                          SkFixedToFloat(texture->normalizeFixedY(ty + th)),
                                          kVertexSize);
    fCurrVertex += 4;
}

inline void GrDistanceFieldTextContext::init(const GrPaint& paint, const SkPaint& skPaint) {
    GrTextContext::init(paint, skPaint);

    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fMaxVertices = 0;

    if (fSkPaint.getTextSize() <= kSmallDFFontLimit) {
        fTextRatio = fSkPaint.getTextSize()/kSmallDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kSmallDFFontSize));
    } else if (fSkPaint.getTextSize() <= kMediumDFFontLimit) {
        fTextRatio = fSkPaint.getTextSize()/kMediumDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kMediumDFFontSize));
    } else {
        fTextRatio = fSkPaint.getTextSize()/kLargeDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kLargeDFFontSize));
    }

    fUseLCDText = fSkPaint.isLCDRenderText();

    fSkPaint.setLCDRenderText(false);
    fSkPaint.setAutohinted(false);
    fSkPaint.setHinting(SkPaint::kNormal_Hinting);
    fSkPaint.setSubpixelText(true);

}

inline void GrDistanceFieldTextContext::finish() {
    flushGlyphs();

    GrTextContext::finish();
}

static void setup_gamma_texture(GrContext* context, const SkGlyphCache* cache,
                                const SkDeviceProperties& deviceProperties,
                                GrTexture** gammaTexture) {
    if (NULL == *gammaTexture) {
        int width, height;
        size_t size;

#ifdef SK_GAMMA_CONTRAST
        SkScalar contrast = SK_GAMMA_CONTRAST;
#else
        SkScalar contrast = 0.5f;
#endif
        SkScalar paintGamma = deviceProperties.fGamma;
        SkScalar deviceGamma = deviceProperties.fGamma;

        size = SkScalerContext::GetGammaLUTSize(contrast, paintGamma, deviceGamma,
                                                &width, &height);

        SkAutoTArray<uint8_t> data((int)size);
        SkScalerContext::GetGammaLUTData(contrast, paintGamma, deviceGamma, data.get());

        // TODO: Update this to use the cache rather than directly creating a texture.
        GrTextureDesc desc;
        desc.fFlags = kDynamicUpdate_GrTextureFlagBit;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fConfig = kAlpha_8_GrPixelConfig;

        *gammaTexture = context->getGpu()->createTexture(desc, NULL, 0);
        if (NULL == *gammaTexture) {
            return;
        }

        context->writeTexturePixels(*gammaTexture,
                                    0, 0, width, height,
                                    (*gammaTexture)->config(), data.get(), 0,
                                    GrContext::kDontFlush_PixelOpsFlag);
    }
}

void GrDistanceFieldTextContext::drawText(const GrPaint& paint, const SkPaint& skPaint,
                                          const char text[], size_t byteLength,
                                          SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw or can't draw
    if (text == NULL || byteLength == 0 /* no raster clip? || fRC->isEmpty()*/
        || fSkPaint.getRasterizer()) {
        return;
    }

    this->init(paint, skPaint);

    SkScalar sizeRatio = fTextRatio;

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCacheNoGamma    autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache*              cache = autoCache.getCache();
    GrFontScaler*              fontScaler = GetGrFontScaler(cache);

    setup_gamma_texture(fContext, cache, fDeviceProperties, &fGammaTexture);

    // need to measure first
    // TODO - generate positions and pre-load cache as well?
    const char* stop = text + byteLength;
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

        SkScalar alignX = SkFixedToScalar(stopX)*sizeRatio;
        SkScalar alignY = SkFixedToScalar(stopY)*sizeRatio;

        if (fSkPaint.getTextAlign() == SkPaint::kCenter_Align) {
            alignX = SkScalarHalf(alignX);
            alignY = SkScalarHalf(alignY);
        }

        x -= alignX;
        y -= alignY;
    }

    SkFixed fx = SkScalarToFixed(x);
    SkFixed fy = SkScalarToFixed(y);
    SkFixed fixedScale = SkScalarToFixed(sizeRatio);
    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        if (glyph.fWidth) {
            this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                glyph.getSubXFixed(),
                                                glyph.getSubYFixed()),
                                  fx,
                                  fy,
                                  fontScaler);
        }

        fx += SkFixedMul_portable(glyph.fAdvanceX, fixedScale);
        fy += SkFixedMul_portable(glyph.fAdvanceY, fixedScale);
    }

    this->finish();
}

void GrDistanceFieldTextContext::drawPosText(const GrPaint& paint, const SkPaint& skPaint,
                                             const char text[], size_t byteLength,
                                             const SkScalar pos[], SkScalar constY,
                                             int scalarsPerPosition) {

    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0 /* no raster clip? || fRC->isEmpty()*/) {
        return;
    }

    this->init(paint, skPaint);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCacheNoGamma    autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache*              cache = autoCache.getCache();
    GrFontScaler*              fontScaler = GetGrFontScaler(cache);

    setup_gamma_texture(fContext, cache, fDeviceProperties, &fGammaTexture);

    const char*        stop = text + byteLength;

    if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
        while (text < stop) {
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = pos[0];
                SkScalar y = scalarsPerPosition == 1 ? constY : pos[1];

                this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      SkScalarToFixed(x),
                                      SkScalarToFixed(y),
                                      fontScaler);
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

                this->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      SkScalarToFixed(x) - (glyph.fAdvanceX >> alignShift),
                                      SkScalarToFixed(y) - (glyph.fAdvanceY >> alignShift),
                                      fontScaler);
            }
            pos += scalarsPerPosition;
        }
    }

    this->finish();
}
