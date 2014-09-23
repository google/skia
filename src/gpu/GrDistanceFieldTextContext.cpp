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

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 64;
static const int kMediumDFFontLimit = 64;
static const int kLargeDFFontSize = 128;

namespace {
// position + texture coord
extern const GrVertexAttrib gTextVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(SkPoint) , kGeometryProcessor_GrVertexAttribBinding}
};

static const size_t kTextVASize = 2 * sizeof(SkPoint); 

// position + color + texture coord
extern const GrVertexAttrib gTextVertexWithColorAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,                                 kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(SkPoint),                   kColor_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType,  sizeof(SkPoint) + sizeof(GrColor), kGeometryProcessor_GrVertexAttribBinding}
};
    
static const size_t kTextVAColorSize = 2 * sizeof(SkPoint) + sizeof(GrColor); 

};

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
    fEffectTextureUniqueID = SK_InvalidUniqueID;
    fEffectColor = GrColor_ILLEGAL;
    fEffectFlags = 0;

    fVertices = NULL;
    fMaxVertices = 0;

    fVertexBounds.setLargestInverted();
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

void GrDistanceFieldTextContext::setupCoverageEffect(const SkColor& filteredColor) {
    GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kBilerp_FilterMode);
    GrTextureParams gammaParams(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
    
    uint32_t textureUniqueID = fCurrTexture->getUniqueID();
    const SkMatrix& ctm = fContext->getMatrix();
    
    // set up any flags
    uint32_t flags = 0;
    flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    flags |= fUseLCDText ? kUseLCD_DistanceFieldEffectFlag : 0;
    flags |= fUseLCDText && ctm.rectStaysRect() ?
    kRectToRect_DistanceFieldEffectFlag : 0;
    bool useBGR = SkPixelGeometryIsBGR(fDeviceProperties.fPixelGeometry);
    flags |= fUseLCDText && useBGR ? kBGR_DistanceFieldEffectFlag : 0;
    
    // see if we need to create a new effect
    if (textureUniqueID != fEffectTextureUniqueID ||
        filteredColor != fEffectColor ||
        flags != fEffectFlags) {
        if (fUseLCDText) {
            GrColor colorNoPreMul = skcolor_to_grcolor_nopremultiply(filteredColor);
            fCachedGeometryProcessor.reset(
                    GrDistanceFieldLCDTextureEffect::Create(fCurrTexture,
                                                            params,
                                                            fGammaTexture,
                                                            gammaParams,
                                                            colorNoPreMul,
                                                            flags));
        } else {
#ifdef SK_GAMMA_APPLY_TO_A8
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(fDeviceProperties.getGamma(),
                                                                filteredColor);
            fCachedGeometryProcessor.reset(
                    GrDistanceFieldTextureEffect::Create(fCurrTexture,
                                                         params,
                                                         fGammaTexture,
                                                         gammaParams,
                                                         lum/255.f,
                                                         flags));
#else
            fCachedGeometryProcessor.reset(GrDistanceFieldTextureEffect::Create(fCurrTexture,
                                                                                params, flags));
#endif
        }
        fEffectTextureUniqueID = textureUniqueID;
        fEffectColor = filteredColor;
        fEffectFlags = flags;
    }
    
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

        // get our current color
        SkColor filteredColor;
        SkColorFilter* colorFilter = fSkPaint.getColorFilter();
        if (colorFilter) {
            filteredColor = colorFilter->filterColor(fSkPaint.getColor());
        } else {
            filteredColor = fSkPaint.getColor();
        }
        this->setupCoverageEffect(filteredColor);
       
        // Effects could be stored with one of the cache objects (atlas?)
        drawState->setGeometryProcessor(fCachedGeometryProcessor.get());
        
        // Set draw state
        if (fUseLCDText) {
            GrColor colorNoPreMul = skcolor_to_grcolor_nopremultiply(filteredColor);
            if (kOne_GrBlendCoeff != fPaint.getSrcBlendCoeff() ||
                kISA_GrBlendCoeff != fPaint.getDstBlendCoeff() ||
                fPaint.numColorStages()) {
                GrPrintf("LCD Text will not draw correctly.\n");
            }
            SkASSERT(!drawState->hasColorVertexAttribute());
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
            // set back to normal in case we took LCD path previously.
            drawState->setBlendFunc(fPaint.getSrcBlendCoeff(), fPaint.getDstBlendCoeff());
            // We're using per-vertex color.
            SkASSERT(drawState->hasColorVertexAttribute());
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
        SkSafeSetNull(fCurrTexture);
        fVertexBounds.setLargestInverted();
    }
}

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
        if (!fStrike->glyphTooLargeForAtlas(glyph)) {
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

    bool useColorVerts = !fUseLCDText;
    
    if (NULL == fVertices) {
        // If we need to reserve vertices allow the draw target to suggest
        // a number of verts to reserve and whether to perform a flush.
        fMaxVertices = kMinRequestedVerts;
        if (useColorVerts) {
            fDrawTarget->drawState()->setVertexAttribs<gTextVertexWithColorAttribs>(
                                                    SK_ARRAY_COUNT(gTextVertexWithColorAttribs),
                                                    kTextVAColorSize);
        } else {
            fDrawTarget->drawState()->setVertexAttribs<gTextVertexAttribs>(
                                                    SK_ARRAY_COUNT(gTextVertexAttribs),
                                                    kTextVASize);
        }
        bool flush = fDrawTarget->geometryHints(&fMaxVertices, NULL);
        if (flush) {
            this->flushGlyphs();
            fContext->flush();
            if (useColorVerts) {
                fDrawTarget->drawState()->setVertexAttribs<gTextVertexWithColorAttribs>(
                                                    SK_ARRAY_COUNT(gTextVertexWithColorAttribs),
                                                    kTextVAColorSize);
            } else {
                fDrawTarget->drawState()->setVertexAttribs<gTextVertexAttribs>(
                                                    SK_ARRAY_COUNT(gTextVertexAttribs),
                                                    kTextVASize);
            }
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
                                                               &fVertices,
                                                               NULL);
        GrAlwaysAssert(success);
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

    SkRect r;
    r.fLeft = sx;
    r.fTop = sy;
    r.fRight = sx + width;
    r.fBottom = sy + height;

    fVertexBounds.growToInclude(r);

    size_t vertSize = fUseLCDText ? (2 * sizeof(SkPoint))
                                  : (2 * sizeof(SkPoint) + sizeof(GrColor));

    SkASSERT(vertSize == fDrawTarget->getDrawState().getVertexStride());

    SkPoint* positions = reinterpret_cast<SkPoint*>(
        reinterpret_cast<intptr_t>(fVertices) + vertSize * fCurrVertex);
    positions->setRectFan(r.fLeft, r.fTop, r.fRight, r.fBottom, vertSize);

    // The texture coords are last in both the with and without color vertex layouts.
    SkPoint* textureCoords = reinterpret_cast<SkPoint*>(
            reinterpret_cast<intptr_t>(positions) + vertSize  - sizeof(SkPoint));
    textureCoords->setRectFan(SkFixedToFloat(texture->normalizeFixedX(tx)),
                              SkFixedToFloat(texture->normalizeFixedY(ty)),
                              SkFixedToFloat(texture->normalizeFixedX(tx + tw)),
                              SkFixedToFloat(texture->normalizeFixedY(ty + th)),
                              vertSize);
    if (useColorVerts) {
        if (0xFF == GrColorUnpackA(fPaint.getColor())) {
            fDrawTarget->drawState()->setHint(GrDrawState::kVertexColorsAreOpaque_Hint, true);
        }
        // color comes after position.
        GrColor* colors = reinterpret_cast<GrColor*>(positions + 1);
        for (int i = 0; i < 4; ++i) {
            *colors = fPaint.getColor();
            colors = reinterpret_cast<GrColor*>(reinterpret_cast<intptr_t>(colors) + vertSize);
        }
    }

    fCurrVertex += 4;
}

inline void GrDistanceFieldTextContext::init(const GrPaint& paint, const SkPaint& skPaint) {
    GrTextContext::init(paint, skPaint);

    fStrike = NULL;

    const SkMatrix& ctm = fContext->getMatrix();

    // getMaxScale doesn't support perspective, so neither do we at the moment
    SkASSERT(!ctm.hasPerspective());
    SkScalar maxScale = ctm.getMaxScale();
    SkScalar textSize = fSkPaint.getTextSize();
    SkScalar scaledTextSize = textSize;
    // if we have non-unity scale, we need to choose our base text size
    // based on the SkPaint's text size multiplied by the max scale factor
    // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
    if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
        scaledTextSize *= maxScale;
    }

    fCurrVertex = 0;

    fVertices = NULL;

    if (scaledTextSize <= kSmallDFFontLimit) {
        fTextRatio = textSize / kSmallDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        fTextRatio = textSize / kMediumDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kMediumDFFontSize));
    } else {
        fTextRatio = textSize / kLargeDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kLargeDFFontSize));
    }

    fUseLCDText = fSkPaint.isLCDRenderText();

    fSkPaint.setLCDRenderText(false);
    fSkPaint.setAutohinted(false);
    fSkPaint.setHinting(SkPaint::kNormal_Hinting);
    fSkPaint.setSubpixelText(true);

}

inline void GrDistanceFieldTextContext::finish() {
    this->flushGlyphs();

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
        SkScalar paintGamma = deviceProperties.getGamma();
        SkScalar deviceGamma = deviceProperties.getGamma();

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
