/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextContext.h"
#include "GrAtlas.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTarget.h"
#include "GrFontCache.h"
#include "GrFontScaler.h"
#include "GrIndexBuffer.h"
#include "GrStrokeInfo.h"
#include "GrTexturePriv.h"

#include "SkAutoKern.h"
#include "SkColorPriv.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGr.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "SkTextMapStateProc.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrSimpleTextureEffect.h"

SK_CONF_DECLARE(bool, c_DumpFontCache, "gpu.dumpFontCache", false,
                "Dump the contents of the font cache before every purge.");

namespace {
static const size_t kLCDTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);

// position + local coord
static const size_t kColorTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);

static const size_t kGrayTextVASize = sizeof(SkPoint) + sizeof(GrColor) + sizeof(SkIPoint16);

static const int kVerticesPerGlyph = 4;
static const int kIndicesPerGlyph = 6;
};

GrBitmapTextContext::GrBitmapTextContext(GrContext* context,
                                         const SkDeviceProperties& properties)
                                       : GrTextContext(context, properties) {
    fStrike = NULL;

    fCurrTexture = NULL;
    fEffectTextureUniqueID = SK_InvalidUniqueID;

    fVertices = NULL;
    fCurrVertex = 0;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;

    fVertexBounds.setLargestInverted();
}

GrBitmapTextContext* GrBitmapTextContext::Create(GrContext* context,
                                                 const SkDeviceProperties& props) {
    return SkNEW_ARGS(GrBitmapTextContext, (context, props));
}

bool GrBitmapTextContext::canDraw(const SkPaint& paint, const SkMatrix& viewMatrix) {
    return !SkDraw::ShouldDrawTextAsPaths(paint, viewMatrix);
}

inline void GrBitmapTextContext::init(GrRenderTarget* rt, const GrClip& clip,
                                      const GrPaint& paint, const SkPaint& skPaint) {
    GrTextContext::init(rt, clip, paint, skPaint);

    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;
}

void GrBitmapTextContext::onDrawText(GrRenderTarget* rt, const GrClip& clip,
                                     const GrPaint& paint, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &viewMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // transform our starting point
    {
        SkPoint loc;
        viewMatrix.mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    // need to measure first
    int numGlyphs;
    if (fSkPaint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector    stopVector;
        numGlyphs = MeasureText(cache, glyphCacheProc, text, byteLength, &stopVector);

        SkScalar    stopX = stopVector.fX;
        SkScalar    stopY = stopVector.fY;

        if (fSkPaint.getTextAlign() == SkPaint::kCenter_Align) {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    } else {
        numGlyphs = fSkPaint.textToGlyphs(text, byteLength, NULL);
    }
    fTotalVertexCount = kVerticesPerGlyph*numGlyphs;

    const char* stop = text + byteLength;

    SkAutoKern autokern;

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    SkScalar halfSampleX, halfSampleY;
    if (cache->isSubpixel()) {
        halfSampleX = halfSampleY = SkFixedToScalar(SkGlyph::kSubpixelRound);
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }
    } else {
        halfSampleX = halfSampleY = SK_ScalarHalf;
    }

    Sk48Dot16 fx = SkScalarTo48Dot16(x + halfSampleX);
    Sk48Dot16 fy = SkScalarTo48Dot16(y + halfSampleY);

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix, but for
    // performance reasons we just invert here instead
    if (!viewMatrix.invert(&fLocalMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

        fx += autokern.adjust(glyph);

        if (glyph.fWidth) {
            this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                            glyph.getSubXFixed(),
                                            glyph.getSubYFixed()),
                              Sk48Dot16FloorToInt(fx),
                              Sk48Dot16FloorToInt(fy),
                              fontScaler);
        }

        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    this->finish();
}

void GrBitmapTextContext::onDrawPosText(GrRenderTarget* rt, const GrClip& clip,
                                        const GrPaint& paint, const SkPaint& skPaint,
                                        const SkMatrix& viewMatrix,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition,
                                        const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == NULL || byteLength == 0/* || fRC->isEmpty()*/) {
        return;
    }

    this->init(rt, clip, paint, skPaint);

    SkDrawCacheProc glyphCacheProc = fSkPaint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(fSkPaint, &fDeviceProperties, &viewMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    GrFontScaler*       fontScaler = GetGrFontScaler(cache);

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix, but for
    // performance reasons we just invert here instead
    if (!viewMatrix.invert(&fLocalMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    int numGlyphs = fSkPaint.textToGlyphs(text, byteLength, NULL);
    fTotalVertexCount = kVerticesPerGlyph*numGlyphs;

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(fSkPaint.getTextAlign());
    SkTextMapStateProc tmsProc(viewMatrix, offset, scalarsPerPosition);
    SkScalar halfSampleX = 0, halfSampleY = 0;

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_ScalarHalf;
        }

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + halfSampleX);
                Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + halfSampleY);

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);
                    SkPoint alignLoc;
                    alignProc(tmsLoc, metricGlyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + halfSampleX);
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + halfSampleY);

                    // have to call again, now that we've been "aligned"
                    const SkGlyph& glyph = glyphCacheProc(cache, &currentText,
                                                          fx & fxMask, fy & fyMask);
                    // the assumption is that the metrics haven't changed
                    SkASSERT(prevAdvX == glyph.fAdvanceX);
                    SkASSERT(prevAdvY == glyph.fAdvanceY);
                    SkASSERT(glyph.fWidth);

                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkPoint alignLoc;
                    alignProc(tmsLoc, glyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + SK_ScalarHalf); //halfSampleX;
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + SK_ScalarHalf); //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                    glyph.getSubXFixed(),
                                                    glyph.getSubYFixed()),
                                      Sk48Dot16FloorToInt(fx),
                                      Sk48Dot16FloorToInt(fy),
                                      fontScaler);
                }
                pos += scalarsPerPosition;
            }
        }
    }

    this->finish();
}

static size_t get_vertex_stride(GrMaskFormat maskFormat) {
    switch (maskFormat) {
        case kA8_GrMaskFormat:
            return kGrayTextVASize;
        case kARGB_GrMaskFormat:
            return kColorTextVASize;
        default:
            return kLCDTextVASize;
    }
}

static void* alloc_vertices(GrDrawTarget* drawTarget,
                            int numVertices,
                            GrMaskFormat maskFormat) {
    if (numVertices <= 0) {
        return NULL;
    }

    // set up attributes
    void* vertices = NULL;
    bool success = drawTarget->reserveVertexAndIndexSpace(numVertices,
                                                          get_vertex_stride(maskFormat),
                                                          0,
                                                          &vertices,
                                                          NULL);
    GrAlwaysAssert(success);
    return vertices;
}

inline bool GrBitmapTextContext::uploadGlyph(GrGlyph* glyph, GrFontScaler* scaler) {
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

void GrBitmapTextContext::appendGlyph(GrGlyph::PackedID packed,
                                      int vx, int vy,
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

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    // check if we clipped out
    if (fClipRect.quickReject(x, y, x + width, y + height)) {
        return;
    }

    // If the glyph is too large we fall back to paths
    if (NULL == glyph->fPlot && !uploadGlyph(glyph, scaler)) {
        if (NULL == glyph->fPath) {
            SkPath* path = SkNEW(SkPath);
            if (!scaler->getGlyphPath(glyph->glyphID(), path)) {
                // flag the glyph as being dead?
                delete path;
                return;
            }
            glyph->fPath = path;
        }

        // flush any accumulated draws before drawing this glyph as a path.
        this->flush();

        SkMatrix translate;
        translate.setTranslate(SkIntToScalar(vx), SkIntToScalar(vy));
        SkPath tmpPath(*glyph->fPath);
        tmpPath.transform(translate);
        GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(fRenderTarget, fClip, fPaint, SkMatrix::I(), tmpPath, strokeInfo);

        // remove this glyph from the vertices we need to allocate
        fTotalVertexCount -= kVerticesPerGlyph;
        return;
    }

    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

    // the current texture/maskformat must match what the glyph needs
    GrTexture* texture = glyph->fPlot->texture();
    SkASSERT(texture);

    if (fCurrTexture != texture || fCurrVertex + kVerticesPerGlyph > fAllocVertexCount) {
        this->flush();
        fCurrTexture = texture;
        fCurrTexture->ref();
        fCurrMaskFormat = glyph->fMaskFormat;
    }

    if (NULL == fVertices) {
        int maxQuadVertices = kVerticesPerGlyph * fContext->getQuadIndexBuffer()->maxQuads();
        fAllocVertexCount = SkMin32(fTotalVertexCount, maxQuadVertices);
        fVertices = alloc_vertices(fDrawTarget, fAllocVertexCount, fCurrMaskFormat);
    }

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    fVertexBounds.joinNonEmptyArg(r);

    int u0 = glyph->fAtlasLocation.fX;
    int v0 = glyph->fAtlasLocation.fY;
    int u1 = u0 + width;
    int v1 = v0 + height;

    size_t vertSize = get_vertex_stride(fCurrMaskFormat);
    intptr_t vertex = reinterpret_cast<intptr_t>(fVertices) + vertSize * fCurrVertex;

    // V0
    SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fTop);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    SkIPoint16* textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize -
                                                              sizeof(SkIPoint16));
    textureCoords->set(u0, v0);
    vertex += vertSize;

    // V1
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fLeft, r.fBottom);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u0, v1);
    vertex += vertSize;

    // V2
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fBottom);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v1);
    vertex += vertSize;

    // V3
    position = reinterpret_cast<SkPoint*>(vertex);
    position->set(r.fRight, r.fTop);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        SkColor* color = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *color = fPaint.getColor();
    }
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex + vertSize  - sizeof(SkIPoint16));
    textureCoords->set(u1, v0);

    fCurrVertex += 4;
}

void GrBitmapTextContext::flush() {
    if (NULL == fDrawTarget) {
        return;
    }

    if (fCurrVertex > 0) {
        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setFromPaint(fPaint, fRenderTarget, fClip);

        // setup our sampler state for our text texture/atlas
        SkASSERT(SkIsAlign4(fCurrVertex));
        SkASSERT(fCurrTexture);

        SkASSERT(fStrike);
        GrColor color = fPaint.getColor();
        switch (fCurrMaskFormat) {
                // Color bitmap text
            case kARGB_GrMaskFormat: {
                int a = fSkPaint.getAlpha();
                color = SkColorSetARGB(a, a, a, a);
                break;
            }
                // LCD text
            case kA565_GrMaskFormat: {
                // TODO: move supportsRGBCoverage check to setupCoverageEffect and only add LCD
                // processor if the xp can support it. For now we will simply assume that if
                // fUseLCDText is true, then we have a known color output.
                const GrXPFactory* xpFactory = pipelineBuilder.getXPFactory();
                if (!xpFactory->supportsRGBCoverage(0, kRGBA_GrColorComponentFlags)) {
                    SkDebugf("LCD Text will not draw correctly.\n");
                }
                break;
            }
                // Grayscale/BW text
            case kA8_GrMaskFormat:
                break;
            default:
                SkFAIL("Unexpected mask format.");
        }

        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
        uint32_t textureUniqueID = fCurrTexture->getUniqueID();
        if (textureUniqueID != fEffectTextureUniqueID ||
            fCachedGeometryProcessor->color() != color ||
            !fCachedGeometryProcessor->localMatrix().cheapEqualTo(fLocalMatrix)) {
            // This will be ignored in the non A8 case
            bool opaqueVertexColors = GrColorIsOpaque(fPaint.getColor());
            fCachedGeometryProcessor.reset(GrBitmapTextGeoProc::Create(color,
                                                                       fCurrTexture,
                                                                       params,
                                                                       fCurrMaskFormat,
                                                                       opaqueVertexColors,
                                                                       fLocalMatrix));
            fEffectTextureUniqueID = textureUniqueID;
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
        fAllocVertexCount = 0;
        // reset to be those that are left
        fTotalVertexCount -= fCurrVertex;
        fCurrVertex = 0;
        fVertexBounds.setLargestInverted();
        SkSafeSetNull(fCurrTexture);
    }
}

inline void GrBitmapTextContext::finish() {
    this->flush();
    fTotalVertexCount = 0;

    GrTextContext::finish();
}

