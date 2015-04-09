/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextContext.h"
#include "GrAtlas.h"
#include "GrAtlasTextContext.h"
#include "GrBitmapTextContext.h"
#include "GrDrawTarget.h"
#include "GrDrawTargetCaps.h"
#include "GrFontAtlasSizes.h"
#include "GrFontCache.h"
#include "GrFontScaler.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrStrokeInfo.h"
#include "GrTexturePriv.h"

#include "SkAutoKern.h"
#include "SkColorFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "effects/GrDistanceFieldGeoProc.h"

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

static const int kMinDFFontSize = 18;
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;

static const int kVerticesPerGlyph = 4;
static const int kIndicesPerGlyph = 6;

#ifdef SK_DEBUG
static const int kExpectedDistanceAdjustTableSize = 8;
#endif
static const int kDistanceAdjustLumShift = 5;

GrDistanceFieldTextContext::GrDistanceFieldTextContext(GrContext* context,
                                                       SkGpuDevice* gpuDevice,
                                                       const SkDeviceProperties& properties,
                                                       bool enable)
    : GrTextContext(context, gpuDevice, properties) {
#if SK_FORCE_DISTANCE_FIELD_TEXT
    fEnableDFRendering = true;
#else
    fEnableDFRendering = enable;
#endif
    fStrike = NULL;
    fDistanceAdjustTable = NULL;

    fEffectTextureUniqueID = SK_InvalidUniqueID;
    fEffectColor = GrColor_ILLEGAL;
    fEffectFlags = kInvalid_DistanceFieldEffectFlag;

    fVertices = NULL;
    fCurrVertex = 0;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;
    fCurrTexture = NULL;

    fVertexBounds.setLargestInverted();
}

GrDistanceFieldTextContext* GrDistanceFieldTextContext::Create(GrContext* context,
                                                               SkGpuDevice* gpuDevice,
                                                               const SkDeviceProperties& props,
                                                               bool enable) {
    GrDistanceFieldTextContext* textContext = SkNEW_ARGS(GrDistanceFieldTextContext, 
                                                         (context, gpuDevice, props, enable));
    textContext->buildDistanceAdjustTable();
#ifdef USE_BITMAP_TEXTBLOBS
    textContext->fFallbackTextContext = GrAtlasTextContext::Create(context, gpuDevice, props);
#else
    textContext->fFallbackTextContext = GrBitmapTextContext::Create(context, gpuDevice, props);
#endif

    return textContext;
}

void GrDistanceFieldTextContext::buildDistanceAdjustTable() {

    // This is used for an approximation of the mask gamma hack, used by raster and bitmap
    // text. The mask gamma hack is based off of guessing what the blend color is going to
    // be, and adjusting the mask so that when run through the linear blend will
    // produce the value closest to the desired result. However, in practice this means
    // that the 'adjusted' mask is just increasing or decreasing the coverage of
    // the mask depending on what it is thought it will blit against. For black (on
    // assumed white) this means that coverages are decreased (on a curve). For white (on
    // assumed black) this means that coverages are increased (on a a curve). At
    // middle (perceptual) gray (which could be blit against anything) the coverages
    // remain the same.
    //
    // The idea here is that instead of determining the initial (real) coverage and
    // then adjusting that coverage, we determine an adjusted coverage directly by
    // essentially manipulating the geometry (in this case, the distance to the glyph
    // edge). So for black (on assumed white) this thins a bit; for white (on
    // assumed black) this fake bolds the geometry a bit.
    //
    // The distance adjustment is calculated by determining the actual coverage value which
    // when fed into in the mask gamma table gives us an 'adjusted coverage' value of 0.5. This
    // actual coverage value (assuming it's between 0 and 1) corresponds to a distance from the
    // actual edge. So by subtracting this distance adjustment and computing without the
    // the coverage adjustment we should get 0.5 coverage at the same point.
    //
    // This has several implications:
    //     For non-gray lcd smoothed text, each subpixel essentially is using a
    //     slightly different geometry.
    //
    //     For black (on assumed white) this may not cover some pixels which were
    //     previously covered; however those pixels would have been only slightly
    //     covered and that slight coverage would have been decreased anyway. Also, some pixels
    //     which were previously fully covered may no longer be fully covered.
    //
    //     For white (on assumed black) this may cover some pixels which weren't
    //     previously covered at all.

    int width, height;
    size_t size;

#ifdef SK_GAMMA_CONTRAST
    SkScalar contrast = SK_GAMMA_CONTRAST;
#else
    SkScalar contrast = 0.5f;
#endif
    SkScalar paintGamma = fDeviceProperties.gamma();
    SkScalar deviceGamma = fDeviceProperties.gamma();

    size = SkScalerContext::GetGammaLUTSize(contrast, paintGamma, deviceGamma,
        &width, &height);
   
    SkASSERT(kExpectedDistanceAdjustTableSize == height);
    fDistanceAdjustTable = SkNEW_ARRAY(SkScalar, height);

    SkAutoTArray<uint8_t> data((int)size);
    SkScalerContext::GetGammaLUTData(contrast, paintGamma, deviceGamma, data.get());

    // find the inverse points where we cross 0.5
    // binsearch might be better, but we only need to do this once on creation
    for (int row = 0; row < height; ++row) {
        uint8_t* rowPtr = data.get() + row*width;
        for (int col = 0; col < width - 1; ++col) {
            if (rowPtr[col] <= 127 && rowPtr[col + 1] >= 128) {
                // compute point where a mask value will give us a result of 0.5
                float interp = (127.5f - rowPtr[col]) / (rowPtr[col + 1] - rowPtr[col]);
                float borderAlpha = (col + interp) / 255.f;

                // compute t value for that alpha
                // this is an approximate inverse for smoothstep()
                float t = borderAlpha*(borderAlpha*(4.0f*borderAlpha - 6.0f) + 5.0f) / 3.0f;

                // compute distance which gives us that t value
                const float kDistanceFieldAAFactor = 0.65f; // should match SK_DistanceFieldAAFactor
                float d = 2.0f*kDistanceFieldAAFactor*t - kDistanceFieldAAFactor;

                fDistanceAdjustTable[row] = d;
                break;
            }
        }
    }
}


GrDistanceFieldTextContext::~GrDistanceFieldTextContext() {
    SkDELETE_ARRAY(fDistanceAdjustTable);
    fDistanceAdjustTable = NULL;
}

bool GrDistanceFieldTextContext::canDraw(const GrRenderTarget* rt,
                                         const GrClip& clip,
                                         const GrPaint& paint,
                                         const SkPaint& skPaint,
                                         const SkMatrix& viewMatrix) {
    // TODO: support perspective (need getMaxScale replacement)
    if (viewMatrix.hasPerspective()) {
        return false;
    }

    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar scaledTextSize = maxScale*skPaint.getTextSize();
    // Hinted text looks far better at small resolutions
    // Scaling up beyond 2x yields undesireable artifacts
    if (scaledTextSize < kMinDFFontSize || scaledTextSize > 2*kLargeDFFontSize) {
        return false;
    }

    if (!fEnableDFRendering && !skPaint.isDistanceFieldTextTEMP() &&
        scaledTextSize < kLargeDFFontSize) {
        return false;
    }

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (skPaint.getRasterizer() || skPaint.getMaskFilter() ||
        !fContext->getTextTarget()->caps()->shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

inline void GrDistanceFieldTextContext::init(GrRenderTarget* rt, const GrClip& clip,
                                             const GrPaint& paint, const SkPaint& skPaint,
                                             const SkIRect& regionClipBounds) {
    GrTextContext::init(rt, clip, paint, skPaint, regionClipBounds);

    fStrike = NULL;

    const SkMatrix& ctm = fViewMatrix;

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

    fVertices = NULL;
    fCurrVertex = 0;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;

    if (scaledTextSize <= kSmallDFFontLimit) {
        fTextRatio = textSize / kSmallDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kSmallDFFontSize));
#if DEBUG_TEXT_SIZE
        fSkPaint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0x7F));
        fPaint.setColor(GrColorPackRGBA(0x00, 0x00, 0x7F, 0xFF));
#endif
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        fTextRatio = textSize / kMediumDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kMediumDFFontSize));
#if DEBUG_TEXT_SIZE
        fSkPaint.setColor(SkColorSetARGB(0xFF, 0x00, 0x3F, 0x00));
        fPaint.setColor(GrColorPackRGBA(0x00, 0x3F, 0x00, 0xFF));
#endif
    } else {
        fTextRatio = textSize / kLargeDFFontSize;
        fSkPaint.setTextSize(SkIntToScalar(kLargeDFFontSize));
#if DEBUG_TEXT_SIZE
        fSkPaint.setColor(SkColorSetARGB(0xFF, 0x7F, 0x00, 0x00));
        fPaint.setColor(GrColorPackRGBA(0x7F, 0x00, 0x00, 0xFF));
#endif
    }

    fUseLCDText = fSkPaint.isLCDRenderText();

    fSkPaint.setLCDRenderText(false);
    fSkPaint.setAutohinted(false);
    fSkPaint.setHinting(SkPaint::kNormal_Hinting);
    fSkPaint.setSubpixelText(true);
    
    // fix for skia:3528
    // if we're scaling up, include any scaling to match text size in the view matrix
    if (fTextRatio > 1.0f) {
        fViewMatrix.preScale(fTextRatio, fTextRatio);
    }
}

void GrDistanceFieldTextContext::onDrawText(GrRenderTarget* rt, const GrClip& clip,
                                            const GrPaint& paint,
                                            const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                            const char text[], size_t byteLength,
                                            SkScalar x, SkScalar y,
                                            const SkIRect& regionClipBounds) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0) {
        return;
    }

    fViewMatrix = viewMatrix;
    SkDrawCacheProc          glyphCacheProc = skPaint.getDrawCacheProc();
    SkAutoGlyphCache         autoCache(skPaint, &fDeviceProperties, NULL);
    SkGlyphCache*            cache = autoCache.getCache();

    SkTArray<SkScalar> positions;

    const char* textPtr = text;
    SkFixed stopX = 0;
    SkFixed stopY = 0;
    SkFixed origin;
    switch (skPaint.getTextAlign()) {
        case SkPaint::kRight_Align: origin = SK_Fixed1; break;
        case SkPaint::kCenter_Align: origin = SK_FixedHalf; break;
        case SkPaint::kLeft_Align: origin = 0; break;
        default: SkFAIL("Invalid paint origin"); return;
    }

    SkAutoKern autokern;
    const char* stop = text + byteLength;
    while (textPtr < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(cache, &textPtr, 0, 0);

        SkFixed width = glyph.fAdvanceX + autokern.adjust(glyph);
        positions.push_back(SkFixedToScalar(stopX + SkFixedMul(origin, width)));

        SkFixed height = glyph.fAdvanceY;
        positions.push_back(SkFixedToScalar(stopY + SkFixedMul(origin, height)));

        stopX += width;
        stopY += height;
    }
    SkASSERT(textPtr == stop);

    // now adjust starting point depending on alignment
    SkScalar alignX = SkFixedToScalar(stopX);
    SkScalar alignY = SkFixedToScalar(stopY);
    if (skPaint.getTextAlign() == SkPaint::kCenter_Align) {
        alignX = SkScalarHalf(alignX);
        alignY = SkScalarHalf(alignY);
    } else if (skPaint.getTextAlign() == SkPaint::kLeft_Align) {
        alignX = 0;
        alignY = 0;
    }
    x -= alignX;
    y -= alignY;
    SkPoint offset = SkPoint::Make(x, y);

    this->onDrawPosText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, positions.begin(),
                        2, offset, regionClipBounds);
}

void GrDistanceFieldTextContext::onDrawPosText(GrRenderTarget* rt, const GrClip& clip,
                                               const GrPaint& paint,
                                               const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[], int scalarsPerPosition,
                                               const SkPoint& offset,
                                               const SkIRect& regionClipBounds) {

    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0 /* no raster clip? || fRC->isEmpty()*/) {
        return;
    }

    fViewMatrix = viewMatrix;
    this->init(rt, clip, paint, skPaint, regionClipBounds);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCacheNoGamma    autoCache(fSkPaint, &fDeviceProperties, NULL);
    SkGlyphCache*              cache = autoCache.getCache();
    GrFontScaler*              fontScaler = GetGrFontScaler(cache);

    int numGlyphs = fSkPaint.textToGlyphs(text, byteLength, NULL);
    fTotalVertexCount = kVerticesPerGlyph*numGlyphs;

    const char*        stop = text + byteLength;
    SkTArray<char>     fallbackTxt;
    SkTArray<SkScalar> fallbackPos;

    if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                if (!this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                     glyph.getSubXFixed(),
                                                     glyph.getSubYFixed(),
                                                     GrGlyph::kDistance_MaskStyle),
                                       x, y, fontScaler)) {
                    // couldn't append, send to fallback
                    fallbackTxt.push_back_n(SkToInt(text-lastText), lastText);
                    fallbackPos.push_back(pos[0]);
                    if (2 == scalarsPerPosition) {
                        fallbackPos.push_back(pos[1]);
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    } else {
        SkScalar alignMul = SkPaint::kCenter_Align == fSkPaint.getTextAlign() ? SK_ScalarHalf
                                                                              : SK_Scalar1;
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                SkScalar advanceX = SkFixedToScalar(glyph.fAdvanceX)*alignMul*fTextRatio;
                SkScalar advanceY = SkFixedToScalar(glyph.fAdvanceY)*alignMul*fTextRatio;

                if (!this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                     glyph.getSubXFixed(),
                                                     glyph.getSubYFixed(),
                                                     GrGlyph::kDistance_MaskStyle),
                                       x - advanceX, y - advanceY, fontScaler)) {
                    // couldn't append, send to fallback
                    fallbackTxt.push_back_n(SkToInt(text-lastText), lastText);
                    fallbackPos.push_back(pos[0]);
                    if (2 == scalarsPerPosition) {
                        fallbackPos.push_back(pos[1]);
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    }

    this->finish();
    
    if (fallbackTxt.count() > 0) {
        fFallbackTextContext->drawPosText(rt, clip, paint, skPaint, viewMatrix,
                                          fallbackTxt.begin(), fallbackTxt.count(),
                                          fallbackPos.begin(), scalarsPerPosition, offset,
                                          regionClipBounds);
    }
}

static inline GrColor skcolor_to_grcolor_nopremultiply(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    return GrColorPackRGBA(r, g, b, 0xff);
}

static size_t get_vertex_stride(bool useColorVerts) {
    return useColorVerts ? (sizeof(SkPoint) + sizeof(GrColor) + sizeof(SkIPoint16)) :
                           (sizeof(SkPoint) + sizeof(SkIPoint16));
}

static void* alloc_vertices(GrDrawTarget* drawTarget,
                            int numVertices,
                            bool useColorVerts) {
    if (numVertices <= 0) {
        return NULL;
    }

    void* vertices = NULL;
    bool success = drawTarget->reserveVertexAndIndexSpace(numVertices,
                                                          get_vertex_stride(useColorVerts),
                                                          0,
                                                          &vertices,
                                                          NULL);
    GrAlwaysAssert(success);
    return vertices;
}

void GrDistanceFieldTextContext::setupCoverageEffect(const SkColor& filteredColor) {
    GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
    GrTextureParams gammaParams(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
    
    uint32_t textureUniqueID = fCurrTexture->getUniqueID();
    const SkMatrix& ctm = fViewMatrix;
    
    // set up any flags
    uint32_t flags = 0;
    flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    flags |= fUseLCDText ? kUseLCD_DistanceFieldEffectFlag : 0;
    flags |= fUseLCDText && ctm.rectStaysRect() ?
    kRectToRect_DistanceFieldEffectFlag : 0;
    bool useBGR = SkPixelGeometryIsBGR(fDeviceProperties.pixelGeometry());
    flags |= fUseLCDText && useBGR ? kBGR_DistanceFieldEffectFlag : 0;

    // fix for skia:3528
    // set the local matrix to correct any text size scaling for gradients et al.
    SkMatrix localMatrix;
    if (fTextRatio > 1.0f) {
        localMatrix.setScale(fTextRatio, fTextRatio);
    } else {
        localMatrix.reset();
    }

    // see if we need to create a new effect
    if (textureUniqueID != fEffectTextureUniqueID ||
        filteredColor != fEffectColor ||
        flags != fEffectFlags ||
        !fCachedGeometryProcessor->viewMatrix().cheapEqualTo(fViewMatrix) ||
        !fCachedGeometryProcessor->localMatrix().cheapEqualTo(localMatrix)) {
        GrColor color = fPaint.getColor();

        if (fUseLCDText) {
            GrColor colorNoPreMul = skcolor_to_grcolor_nopremultiply(filteredColor);

            float redCorrection = 
                fDistanceAdjustTable[GrColorUnpackR(colorNoPreMul) >> kDistanceAdjustLumShift];
            float greenCorrection = 
                fDistanceAdjustTable[GrColorUnpackG(colorNoPreMul) >> kDistanceAdjustLumShift];
            float blueCorrection = 
                fDistanceAdjustTable[GrColorUnpackB(colorNoPreMul) >> kDistanceAdjustLumShift];
            GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(redCorrection,
                                                                    greenCorrection,
                                                                    blueCorrection);
            fCachedGeometryProcessor.reset(GrDistanceFieldLCDTextGeoProc::Create(color,
                                                                                 fViewMatrix,
                                                                                 localMatrix,
                                                                                 fCurrTexture,
                                                                                 params,
                                                                                 widthAdjust,
                                                                                 flags));
        } else {
            flags |= kColorAttr_DistanceFieldEffectFlag;
            bool opaque = GrColorIsOpaque(color);
#ifdef SK_GAMMA_APPLY_TO_A8
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(fDeviceProperties.gamma(),
                                                                filteredColor);
            float correction = fDistanceAdjustTable[lum >> kDistanceAdjustLumShift];
            fCachedGeometryProcessor.reset(GrDistanceFieldA8TextGeoProc::Create(color,
                                                                                fViewMatrix,
                                                                                localMatrix,
                                                                                fCurrTexture,
                                                                                params,
                                                                                correction,
                                                                                flags,
                                                                                opaque));
#else
            fCachedGeometryProcessor.reset(GrDistanceFieldA8TextGeoProc::Create(color,
                                                                                fViewMatrix,
                                                                                localMatrix,
                                                                                fCurrTexture,
                                                                                params,
                                                                                flags,
                                                                                opaque));
#endif
        }
        fEffectTextureUniqueID = textureUniqueID;
        fEffectColor = filteredColor;
        fEffectFlags = flags;
    }
    
}

inline bool GrDistanceFieldTextContext::uploadGlyph(GrGlyph* glyph, GrFontScaler* scaler) {
    if (!fStrike->glyphTooLargeForAtlas(glyph)) {
        if (fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }
        
        // try to clear out an unused plot before we flush
        if (fContext->getFontCache()->freeUnusedPlot(fStrike, glyph) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }
        
        if (c_DumpFontCache) {
#ifdef SK_DEVELOPER
            fContext->getFontCache()->dump();
#endif
        }
        
        // before we purge the cache, we must flush any accumulated draws
        this->flush();
        fContext->flush();
        
        // we should have an unused plot now
        if (fContext->getFontCache()->freeUnusedPlot(fStrike, glyph) &&
            fStrike->addGlyphToAtlas(glyph, scaler)) {
            return true;
        }
        
        // we should never get here
        SkASSERT(false);
    }
    
    return false;
}


// Returns true if this method handled the glyph, false if needs to be passed to fallback
//
bool GrDistanceFieldTextContext::appendGlyph(GrGlyph::PackedID packed,
                                             SkScalar sx, SkScalar sy,
                                             GrFontScaler* scaler) {
    if (NULL == fDrawTarget) {
        return true;
    }

    if (NULL == fStrike) {
        fStrike = fContext->getFontCache()->getStrike(scaler);
    }

    GrGlyph* glyph = fStrike->getGlyph(packed, scaler);
    if (NULL == glyph || glyph->fBounds.isEmpty()) {
        return true;
    }

    // fallback to color glyph support
    if (kA8_GrMaskFormat != glyph->fMaskFormat) {
        return false;
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2*SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2*SK_DistanceFieldInset);

    SkScalar scale = fTextRatio;
    // if we're scaling up, using fix for skia:3528
    if (scale > 1.0f) {
        sx /= scale;
        sy /= scale;
    } else {
        dx *= scale;
        dy *= scale;
        width *= scale;
        height *= scale;
    }
    sx += dx;
    sy += dy;
    SkRect glyphRect = SkRect::MakeXYWH(sx, sy, width, height);

    // check if we clipped out
    SkRect dstRect;
    const SkMatrix& ctm = fViewMatrix;
    (void) ctm.mapRect(&dstRect, glyphRect);
    if (fClipRect.quickReject(SkScalarTruncToInt(dstRect.left()),
                              SkScalarTruncToInt(dstRect.top()),
                              SkScalarTruncToInt(dstRect.right()),
                              SkScalarTruncToInt(dstRect.bottom()))) {
        return true;
    }

    if (NULL == glyph->fPlot) {
        // needs to be a separate conditional to avoid over-optimization
        // on Nexus 7 and Nexus 10

        // If the glyph is too large we fall back to paths
        if (!uploadGlyph(glyph, scaler)) {
            if (NULL == glyph->fPath) {
                SkPath* path = SkNEW(SkPath);
                if (!scaler->getGlyphPath(glyph->glyphID(), path)) {
                    // flag the glyph as being dead?
                    delete path;
                    return true;
                }
                glyph->fPath = path;
            }

            // flush any accumulated draws before drawing this glyph as a path.
            this->flush();

            SkMatrix ctm;
            ctm.postTranslate(sx - dx, sy - dy);

            SkPath tmpPath(*glyph->fPath);
            tmpPath.transform(ctm);

            GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
            fContext->drawPath(fRenderTarget, fClip, fPaint, fViewMatrix, tmpPath, strokeInfo);

            // remove this glyph from the vertices we need to allocate
            fTotalVertexCount -= kVerticesPerGlyph;
            return true;
        }
    }

    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

    GrTexture* texture = glyph->fPlot->texture();
    SkASSERT(texture);

    if (fCurrTexture != texture || fCurrVertex + kVerticesPerGlyph > fTotalVertexCount) {
        this->flush();
        fCurrTexture = texture;
        fCurrTexture->ref();
    }

    bool useColorVerts = !fUseLCDText;

    if (NULL == fVertices) {
        int maxQuadVertices = kVerticesPerGlyph * fContext->getQuadIndexBuffer()->maxQuads();
        fAllocVertexCount = SkMin32(fTotalVertexCount, maxQuadVertices);
        fVertices = alloc_vertices(fDrawTarget,
                                   fAllocVertexCount,
                                   useColorVerts);
    }

    fVertexBounds.joinNonEmptyArg(glyphRect);

    int u0 = glyph->fAtlasLocation.fX + SK_DistanceFieldInset;
    int v0 = glyph->fAtlasLocation.fY + SK_DistanceFieldInset;
    int u1 = u0 + glyph->fBounds.width() - 2*SK_DistanceFieldInset;
    int v1 = v0 + glyph->fBounds.height() - 2*SK_DistanceFieldInset;

    size_t vertSize = get_vertex_stride(useColorVerts);
    intptr_t vertex = reinterpret_cast<intptr_t>(fVertices) + vertSize * fCurrVertex;

    // V0
    SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
    position->set(glyphRect.fLeft, glyphRect.fTop);
    if (useColorVerts) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    SkIPoint16* textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize -
                                                              sizeof(SkIPoint16));
    textureCoords->set(u0, v0);
    vertex += vertSize;

    // V1
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(glyphRect.fLeft, glyphRect.fBottom);
    if (useColorVerts) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u0, v1);
    vertex += vertSize;

    // V2
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(glyphRect.fRight, glyphRect.fBottom);
    if (useColorVerts) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v1);
    vertex += vertSize;

    // V3
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(glyphRect.fRight, glyphRect.fTop);
    if (useColorVerts) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v0);

    fCurrVertex += 4;
    
    return true;
}

void GrDistanceFieldTextContext::flush() {
    if (NULL == fDrawTarget) {
        return;
    }

    if (fCurrVertex > 0) {
        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setFromPaint(fPaint, fRenderTarget, fClip);

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

        // Set draw state
        if (fUseLCDText) {
            // TODO: move supportsRGBCoverage check to setupCoverageEffect and only add LCD
            // processor if the xp can support it. For now we will simply assume that if
            // fUseLCDText is true, then we have a known color output.
            const GrXPFactory* xpFactory = pipelineBuilder.getXPFactory();
            if (!xpFactory->supportsRGBCoverage(0, kRGBA_GrColorComponentFlags)) {
                SkDebugf("LCD Text will not draw correctly.\n");
            }
            SkASSERT(!fCachedGeometryProcessor->hasVertexColor());
        } else {
            // We're using per-vertex color.
            SkASSERT(fCachedGeometryProcessor->hasVertexColor());
        }
        int nGlyphs = fCurrVertex / kVerticesPerGlyph;
        fDrawTarget->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
        fDrawTarget->drawIndexedInstances(&pipelineBuilder,
                                          fCachedGeometryProcessor.get(),
                                          kTriangles_GrPrimitiveType,
                                          nGlyphs,
                                          kVerticesPerGlyph,
                                          kIndicesPerGlyph,
                                          &fVertexBounds);
        fDrawTarget->resetVertexSource();
        fVertices = NULL;
        fTotalVertexCount -= fCurrVertex;
        fCurrVertex = 0;
        SkSafeSetNull(fCurrTexture);
        fVertexBounds.setLargestInverted();
    }
}

inline void GrDistanceFieldTextContext::finish() {
    this->flush();
    fTotalVertexCount = 0;

    GrTextContext::finish();
}

