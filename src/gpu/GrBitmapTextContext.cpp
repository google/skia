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
static const size_t kLCDTextVASize = 2 * sizeof(SkPoint);

// position + local coord
static const size_t kColorTextVASize = 2 * sizeof(SkPoint);

static const size_t kGrayTextVASize = 2 * sizeof(SkPoint) + sizeof(GrColor);

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

inline void GrBitmapTextContext::init(const GrPaint& paint, const SkPaint& skPaint) {
    GrTextContext::init(paint, skPaint);

    fStrike = NULL;

    fCurrTexture = NULL;
    fCurrVertex = 0;

    fVertices = NULL;
    fAllocVertexCount = 0;
    fTotalVertexCount = 0;
}

void GrBitmapTextContext::onDrawText(const GrPaint& paint, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 /*|| fRC->isEmpty()*/) {
        return;
    }

    this->init(paint, skPaint);

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
    SkFixed halfSampleX, halfSampleY;
    if (cache->isSubpixel()) {
        halfSampleX = halfSampleY = (SK_FixedHalf >> SkGlyph::kSubBits);
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);
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
                              SkFixedFloorToFixed(fx),
                              SkFixedFloorToFixed(fy),
                              fontScaler);
        }

        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    this->finish();
}

void GrBitmapTextContext::onDrawPosText(const GrPaint& paint, const SkPaint& skPaint,
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

    this->init(paint, skPaint);

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
    SkFixed halfSampleX = 0, halfSampleY = 0;

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(viewMatrix);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            halfSampleY = SK_FixedHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            halfSampleX = SK_FixedHalf;
        }

        if (SkPaint::kLeft_Align == fSkPaint.getTextAlign()) {
            while (text < stop) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkFixed fx = SkScalarToFixed(tmsLoc.fX) + halfSampleX;
                SkFixed fy = SkScalarToFixed(tmsLoc.fY) + halfSampleY;

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);
                    SkIPoint fixedLoc;
                    alignProc(tmsLoc, metricGlyph, &fixedLoc);

                    SkFixed fx = fixedLoc.fX + halfSampleX;
                    SkFixed fy = fixedLoc.fY + halfSampleY;

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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkFixed fx = SkScalarToFixed(tmsLoc.fX) + SK_FixedHalf; //halfSampleX;
                    SkFixed fy = SkScalarToFixed(tmsLoc.fY) + SK_FixedHalf; //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkIPoint fixedLoc;
                    alignProc(tmsLoc, glyph, &fixedLoc);

                    SkFixed fx = fixedLoc.fX + SK_FixedHalf; //halfSampleX;
                    SkFixed fy = fixedLoc.fY + SK_FixedHalf; //halfSampleY;
                    this->appendGlyph(GrGlyph::Pack(glyph.getGlyphID(),
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
                                      SkFixed vx, SkFixed vy,
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
    SkFixed width = glyph->fBounds.width();
    SkFixed height = glyph->fBounds.height();

    // check if we clipped out
    if (true || NULL == glyph->fPlot) {
        int x = vx >> 16;
        int y = vy >> 16;
        if (fClipRect.quickReject(x, y, x + width, y + height)) {
//            SkCLZ(3);    // so we can set a break-point in the debugger
            return;
        }
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
        translate.setTranslate(SkFixedToScalar(vx - SkIntToFixed(glyph->fBounds.fLeft)),
                               SkFixedToScalar(vy - SkIntToFixed(glyph->fBounds.fTop)));
        SkPath tmpPath(*glyph->fPath);
        tmpPath.transform(translate);
        GrStrokeInfo strokeInfo(SkStrokeRec::kFill_InitStyle);
        fContext->drawPath(fPaint, SkMatrix::I(), tmpPath, strokeInfo);

        // remove this glyph from the vertices we need to allocate
        fTotalVertexCount -= kVerticesPerGlyph;
        return;
    }

    SkASSERT(glyph->fPlot);
    GrDrawTarget::DrawToken drawToken = fDrawTarget->getCurrentDrawToken();
    glyph->fPlot->setDrawToken(drawToken);

    // now promote them to fixed (TODO: Rethink using fixed pt).
    width = SkIntToFixed(width);
    height = SkIntToFixed(height);

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

    SkFixed tx = SkIntToFixed(glyph->fAtlasLocation.fX);
    SkFixed ty = SkIntToFixed(glyph->fAtlasLocation.fY);

    SkRect r;
    r.fLeft = SkFixedToFloat(vx);
    r.fTop = SkFixedToFloat(vy);
    r.fRight = SkFixedToFloat(vx + width);
    r.fBottom = SkFixedToFloat(vy + height);

    fVertexBounds.joinNonEmptyArg(r);

    size_t vertSize = get_vertex_stride(fCurrMaskFormat);

    SkPoint* positions = reinterpret_cast<SkPoint*>(
        reinterpret_cast<intptr_t>(fVertices) + vertSize * fCurrVertex);
    positions->setRectFan(r.fLeft, r.fTop, r.fRight, r.fBottom, vertSize);

    // The texture coords are last in both the with and without color vertex layouts.
    SkPoint* textureCoords = reinterpret_cast<SkPoint*>(
            reinterpret_cast<intptr_t>(positions) + vertSize  - sizeof(SkPoint));
    textureCoords->setRectFan(SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx + width)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty + height)),
                              vertSize);
    if (kA8_GrMaskFormat == fCurrMaskFormat) {
        // color comes after position.
        GrColor* colors = reinterpret_cast<GrColor*>(positions + 1);
        for (int i = 0; i < 4; ++i) {
           *colors = fPaint.getColor();
           colors = reinterpret_cast<GrColor*>(reinterpret_cast<intptr_t>(colors) + vertSize);
        }
    }
    fCurrVertex += 4;
}

void GrBitmapTextContext::flush() {
    if (NULL == fDrawTarget) {
        return;
    }

    if (fCurrVertex > 0) {
        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setFromPaint(fPaint, fContext->getRenderTarget());

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

        GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kNone_FilterMode);
        if (kARGB_GrMaskFormat == fCurrMaskFormat) {
            uint32_t textureUniqueID = fCurrTexture->getUniqueID();
            if (textureUniqueID != fEffectTextureUniqueID ||
                fCachedGeometryProcessor->color() != color) {
                uint32_t flags = GrDefaultGeoProcFactory::kLocalCoord_GPType;
                fCachedGeometryProcessor.reset(GrDefaultGeoProcFactory::Create(flags, color));
                fCachedTextureProcessor.reset(GrSimpleTextureEffect::Create(fCurrTexture,
                                                                            SkMatrix::I(),
                                                                            params));
                fEffectTextureUniqueID = textureUniqueID;
            }
            pipelineBuilder.addColorProcessor(fCachedTextureProcessor.get());
        } else {
            uint32_t textureUniqueID = fCurrTexture->getUniqueID();
            if (textureUniqueID != fEffectTextureUniqueID ||
                fCachedGeometryProcessor->color() != color ||
                !fCachedGeometryProcessor->localMatrix().cheapEqualTo(fLocalMatrix)) {
                bool hasColor = kA8_GrMaskFormat == fCurrMaskFormat;
                bool opaqueVertexColors = GrColorIsOpaque(fPaint.getColor());
                fCachedGeometryProcessor.reset(GrBitmapTextGeoProc::Create(color,
                                                                           fCurrTexture,
                                                                           params,
                                                                           hasColor,
                                                                           opaqueVertexColors,
                                                                           fLocalMatrix));
                fEffectTextureUniqueID = textureUniqueID;
            }
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

