/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"

#include "GrBlurUtils.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrStrokeInfo.h"
#include "GrTextBlobCache.h"
#include "GrTexturePriv.h"
#include "GrVertexBuffer.h"

#include "SkAutoKern.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGrPriv.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "SkTextBlob.h"
#include "SkTextMapStateProc.h"

#include "batches/GrAtlasTextBatch.h"

namespace {
static const int kMinDFFontSize = 18;
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;
#ifdef SK_BUILD_FOR_ANDROID
static const int kLargeDFFontLimit = 384;
#else
static const int kLargeDFFontLimit = 2 * kLargeDFFontSize;
#endif

SkDEBUGCODE(static const int kExpectedDistanceAdjustTableSize = 8;)
};

GrAtlasTextContext::GrAtlasTextContext(GrContext* context, const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps)
    , fDistanceAdjustTable(new DistanceAdjustTable) {
    // We overallocate vertices in our textblobs based on the assumption that A8 has the greatest
    // vertexStride
    static_assert(GrAtlasTextBatch::kGrayTextVASize >= GrAtlasTextBatch::kColorTextVASize &&
                  GrAtlasTextBatch::kGrayTextVASize >= GrAtlasTextBatch::kLCDTextVASize,
                  "vertex_attribute_changed");
    fCurrStrike = nullptr;
    fCache = context->getTextBlobCache();
}

void GrAtlasTextContext::DistanceAdjustTable::buildDistanceAdjustTable() {

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
    SkScalar paintGamma = SK_GAMMA_EXPONENT;
    SkScalar deviceGamma = SK_GAMMA_EXPONENT;

    size = SkScalerContext::GetGammaLUTSize(contrast, paintGamma, deviceGamma,
        &width, &height);

    SkASSERT(kExpectedDistanceAdjustTableSize == height);
    fTable = new SkScalar[height];

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

                fTable[row] = d;
                break;
            }
        }
    }
}

GrAtlasTextContext* GrAtlasTextContext::Create(GrContext* context,
                                               const SkSurfaceProps& surfaceProps) {
    return new GrAtlasTextContext(context, surfaceProps);
}

bool GrAtlasTextContext::canDraw(const SkPaint& skPaint, const SkMatrix& viewMatrix) {
    return this->canDrawAsDistanceFields(skPaint, viewMatrix) ||
           !SkDraw::ShouldDrawTextAsPaths(skPaint, viewMatrix);
}

GrColor GrAtlasTextContext::ComputeCanonicalColor(const SkPaint& paint, bool lcd) {
    GrColor canonicalColor = paint.computeLuminanceColor();
    if (lcd) {
        // This is the correct computation, but there are tons of cases where LCD can be overridden.
        // For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these overrides are and see if we can incorporate that logic
        // at a higher level *OR* use sRGB
        SkASSERT(false);
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);
    } else {
        // A8, though can have mixed BMP text but it shouldn't matter because BMP text won't have
        // gamma corrected masks anyways, nor color
        U8CPU lum = SkComputeLuminance(SkColorGetR(canonicalColor),
                                       SkColorGetG(canonicalColor),
                                       SkColorGetB(canonicalColor));
        // reduce to our finite number of bits
        canonicalColor = SkMaskGamma::CanonicalColor(SkColorSetRGB(lum, lum, lum));
    }
    return canonicalColor;
}

// TODO if this function ever shows up in profiling, then we can compute this value when the
// textblob is being built and cache it.  However, for the time being textblobs mostly only have 1
// run so this is not a big deal to compute here.
bool GrAtlasTextContext::HasLCD(const SkTextBlob* blob) {
    SkTextBlobRunIterator it(blob);
    for (; !it.done(); it.next()) {
        if (it.isLCD()) {
            return true;
        }
    }
    return false;
}

bool GrAtlasTextContext::MustRegenerateBlob(SkScalar* outTransX, SkScalar* outTransY,
                                            const GrAtlasTextBlob& blob, const SkPaint& paint,
                                            GrColor color, const SkMaskFilter::BlurRec& blurRec,
                                            const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (blob.fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        blob.fPaintColor != color) {
        return true;
    }

    if (blob.fViewMatrix.hasPerspective() != viewMatrix.hasPerspective()) {
        return true;
    }

    if (blob.fViewMatrix.hasPerspective() && !blob.fViewMatrix.cheapEqualTo(viewMatrix)) {
        return true;
    }

    // We only cache one masked version
    if (blob.fKey.fHasBlur &&
        (blob.fBlurRec.fSigma != blurRec.fSigma ||
         blob.fBlurRec.fStyle != blurRec.fStyle ||
         blob.fBlurRec.fQuality != blurRec.fQuality)) {
        return true;
    }

    // Similarly, we only cache one version for each style
    if (blob.fKey.fStyle != SkPaint::kFill_Style &&
        (blob.fStrokeInfo.fFrameWidth != paint.getStrokeWidth() ||
         blob.fStrokeInfo.fMiterLimit != paint.getStrokeMiter() ||
         blob.fStrokeInfo.fJoin != paint.getStrokeJoin())) {
        return true;
    }

    // Mixed blobs must be regenerated.  We could probably figure out a way to do integer scrolls
    // for mixed blobs if this becomes an issue.
    if (blob.hasBitmap() && blob.hasDistanceField()) {
        // Identical viewmatrices and we can reuse in all cases
        if (blob.fViewMatrix.cheapEqualTo(viewMatrix) && x == blob.fX && y == blob.fY) {
            return false;
        }
        return true;
    }

    if (blob.hasBitmap()) {
        if (blob.fViewMatrix.getScaleX() != viewMatrix.getScaleX() ||
            blob.fViewMatrix.getScaleY() != viewMatrix.getScaleY() ||
            blob.fViewMatrix.getSkewX() != viewMatrix.getSkewX() ||
            blob.fViewMatrix.getSkewY() != viewMatrix.getSkewY()) {
            return true;
        }

        // We can update the positions in the cachedtextblobs without regenerating the whole blob,
        // but only for integer translations.
        // This cool bit of math will determine the necessary translation to apply to the already
        // generated vertex coordinates to move them to the correct position
        SkScalar transX = viewMatrix.getTranslateX() +
                          viewMatrix.getScaleX() * (x - blob.fX) +
                          viewMatrix.getSkewX() * (y - blob.fY) -
                          blob.fViewMatrix.getTranslateX();
        SkScalar transY = viewMatrix.getTranslateY() +
                          viewMatrix.getSkewY() * (x - blob.fX) +
                          viewMatrix.getScaleY() * (y - blob.fY) -
                          blob.fViewMatrix.getTranslateY();
        if (!SkScalarIsInt(transX) || !SkScalarIsInt(transY) ) {
            return true;
        }

        (*outTransX) = transX;
        (*outTransY) = transY;
    } else if (blob.hasDistanceField()) {
        // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
        // distance field being generated, so we have to regenerate in those cases
        SkScalar newMaxScale = viewMatrix.getMaxScale();
        SkScalar oldMaxScale = blob.fViewMatrix.getMaxScale();
        SkScalar scaleAdjust = newMaxScale / oldMaxScale;
        if (scaleAdjust < blob.fMaxMinScale || scaleAdjust > blob.fMinMaxScale) {
            return true;
        }

        (*outTransX) = x - blob.fX;
        (*outTransY) = y - blob.fY;
    }

    // It is possible that a blob has neither distanceField nor bitmaptext.  This is in the case
    // when all of the runs inside the blob are drawn as paths.  In this case, we always regenerate
    // the blob anyways at flush time, so no need to regenerate explicitly
    return false;
}


inline SkGlyphCache* GrAtlasTextContext::setupCache(GrAtlasTextBlob::Run* run,
                                                    const SkPaint& skPaint,
                                                    const SkMatrix* viewMatrix,
                                                    bool noGamma) {
    skPaint.getScalerContextDescriptor(&run->fDescriptor, fSurfaceProps, viewMatrix, noGamma);
    run->fTypeface.reset(SkSafeRef(skPaint.getTypeface()));
    return SkGlyphCache::DetachCache(run->fTypeface, run->fDescriptor.getDesc());
}

void GrAtlasTextContext::drawTextBlob(GrDrawContext* dc, GrRenderTarget* rt,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                      SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (fContext->abandoned()) {
        return;
    }

    SkAutoTUnref<GrAtlasTextBlob> cacheBlob;
    SkMaskFilter::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !mf->asABlur(&blurRec)) ||
                      drawFilter);

    if (canCache) {
        bool hasLCD = HasLCD(blob);

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? fSurfaceProps.pixelGeometry() :
                                                 kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                          ComputeCanonicalColor(skPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = blob->uniqueID();
        key.fStyle = skPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        cacheBlob.reset(SkSafeRef(fCache->find(key)));
    }

    SkScalar transX = 0.f;
    SkScalar transY = 0.f;

    // Though for the time being runs in the textblob can override the paint, they only touch font
    // info.
    GrPaint grPaint;
    if (!SkPaintToGrPaint(fContext, skPaint, viewMatrix, &grPaint)) {
        return;
    }

    if (cacheBlob) {
        if (MustRegenerateBlob(&transX, &transY, *cacheBlob, skPaint, grPaint.getColor(), blurRec,
                               viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            fCache->remove(cacheBlob);
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBatch::kGrayTextVASize)));
            this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                     blob, x, y, drawFilter, clip);
        } else {
            // If we can reuse the blob, then make sure we update the blob's viewmatrix, and x/y
            // offsets.  Note, we offset the vertex bounds right before flushing
            cacheBlob->fViewMatrix = viewMatrix;
            cacheBlob->fX = x;
            cacheBlob->fY = y;
            fCache->makeMRU(cacheBlob);
#ifdef CACHE_SANITY_CHECK
            {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                SkAutoTUnref<GrAtlasTextBlob> sanityBlob(fCache->createBlob(glyphCount, runCount,
                                                                            kGrayTextVASize));
                GrTextBlobCache::SetupCacheBlobKey(sanityBlob, key, blurRec, skPaint);
                this->regenerateTextBlob(sanityBlob, skPaint, grPaint.getColor(), viewMatrix,
                                         blob, x, y, drawFilter, clip);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }

#endif
        }
    } else {
        if (canCache) {
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBatch::kGrayTextVASize)));
        } else {
            cacheBlob.reset(fCache->createBlob(blob, GrAtlasTextBatch::kGrayTextVASize));
        }
        this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                 blob, x, y, drawFilter, clip);
    }

    this->flush(blob, cacheBlob, dc, rt, skPaint, grPaint, drawFilter,
                clip, viewMatrix, clipBounds, x, y, transX, transY);
}

inline bool GrAtlasTextContext::canDrawAsDistanceFields(const SkPaint& skPaint,
                                                        const SkMatrix& viewMatrix) {
    // TODO: support perspective (need getMaxScale replacement)
    if (viewMatrix.hasPerspective()) {
        return false;
    }

    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar scaledTextSize = maxScale*skPaint.getTextSize();
    // Hinted text looks far better at small resolutions
    // Scaling up beyond 2x yields undesireable artifacts
    if (scaledTextSize < kMinDFFontSize || scaledTextSize > kLargeDFFontLimit) {
        return false;
    }

    bool useDFT = fSurfaceProps.isUseDeviceIndependentFonts();
#if SK_FORCE_DISTANCE_FIELD_TEXT
    useDFT = true;
#endif

    if (!useDFT && scaledTextSize < kLargeDFFontSize) {
        return false;
    }

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (skPaint.getRasterizer() || skPaint.getMaskFilter() ||
        !fContext->caps()->shaderCaps()->shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrAtlasTextContext::regenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            const SkPaint& skPaint, GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                                            SkDrawFilter* drawFilter,
                                            const GrClip& clip) {
    // The color here is the GrPaint color, and it is used to determine whether we
    // have to regenerate LCD text blobs.
    // We use this color vs the SkPaint color because it has the colorfilter applied.
    cacheBlob->fPaintColor = color;
    cacheBlob->fViewMatrix = viewMatrix;
    cacheBlob->fX = x;
    cacheBlob->fY = y;

    // Regenerate textblob
    SkPaint runPaint = skPaint;
    SkTextBlobRunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(FilterTextFlags(fSurfaceProps, runPaint));

        // setup vertex / glyphIndex for the new run
        if (run > 0) {
            PerSubRunInfo& newRun = cacheBlob->fRuns[run].fSubRunInfo.back();
            PerSubRunInfo& lastRun = cacheBlob->fRuns[run - 1].fSubRunInfo.back();

            newRun.fVertexStartIndex = lastRun.fVertexEndIndex;
            newRun.fVertexEndIndex = lastRun.fVertexEndIndex;

            newRun.fGlyphStartIndex = lastRun.fGlyphEndIndex;
            newRun.fGlyphEndIndex = lastRun.fGlyphEndIndex;
        }

        if (this->canDrawAsDistanceFields(runPaint, viewMatrix)) {
            cacheBlob->setHasDistanceField();
            SkPaint dfPaint = runPaint;
            SkScalar textRatio;
            this->initDistanceFieldPaint(cacheBlob, &dfPaint, &textRatio, viewMatrix);
            Run& runIdx = cacheBlob->fRuns[run];
            PerSubRunInfo& subRun = runIdx.fSubRunInfo.back();
            subRun.fUseLCDText = runPaint.isLCDRenderText();
            subRun.fDrawAsDistanceFields = true;

            SkTDArray<char> fallbackTxt;
            SkTDArray<SkScalar> fallbackPos;
            SkPoint dfOffset;
            int scalarsPerPosition = 2;
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    this->internalDrawDFText(cacheBlob, run, dfPaint, color, viewMatrix,
                                             (const char *)it.glyphs(), textLen,
                                             x + offset.x(), y + offset.y(), textRatio,
                                             &fallbackTxt, &fallbackPos, &dfOffset, runPaint);
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    scalarsPerPosition = 1;
                    dfOffset = SkPoint::Make(x, y + offset.y());
                    this->internalDrawDFPosText(cacheBlob, run, dfPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(),
                                                scalarsPerPosition, dfOffset, textRatio,
                                                &fallbackTxt, &fallbackPos);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    dfOffset = SkPoint::Make(x, y);
                    this->internalDrawDFPosText(cacheBlob, run, dfPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(),
                                                scalarsPerPosition, dfOffset, textRatio,
                                                &fallbackTxt, &fallbackPos);
                    break;
                }
            }
            if (fallbackTxt.count()) {
                this->fallbackDrawPosText(cacheBlob, run, clip, color, runPaint, viewMatrix,
                                          fallbackTxt, fallbackPos, scalarsPerPosition, dfOffset);
            }
        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            cacheBlob->fRuns[run].fDrawAsPaths = true;
        } else {
            cacheBlob->setHasBitmap();
            SkGlyphCache* cache = this->setupCache(&cacheBlob->fRuns[run], runPaint, &viewMatrix,
                                                   false);
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    this->internalDrawBMPText(cacheBlob, run, cache, runPaint, color, viewMatrix,
                                              (const char *)it.glyphs(), textLen,
                                              x + offset.x(), y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    this->internalDrawBMPPosText(cacheBlob, run, cache, runPaint, color, viewMatrix,
                                                 (const char*)it.glyphs(), textLen, it.pos(), 1,
                                                 SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    this->internalDrawBMPPosText(cacheBlob, run, cache, runPaint, color, viewMatrix,
                                                 (const char*)it.glyphs(), textLen, it.pos(), 2,
                                                 SkPoint::Make(x, y));
                    break;
            }
            SkGlyphCache::AttachCache(cache);
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
}

inline void GrAtlasTextContext::initDistanceFieldPaint(GrAtlasTextBlob* blob,
                                                       SkPaint* skPaint,
                                                       SkScalar* textRatio,
                                                       const SkMatrix& viewMatrix) {
    // getMaxScale doesn't support perspective, so neither do we at the moment
    SkASSERT(!viewMatrix.hasPerspective());
    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar textSize = skPaint->getTextSize();
    SkScalar scaledTextSize = textSize;
    // if we have non-unity scale, we need to choose our base text size
    // based on the SkPaint's text size multiplied by the max scale factor
    // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
    if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
        scaledTextSize *= maxScale;
    }

    // We have three sizes of distance field text, and within each size 'bucket' there is a floor
    // and ceiling.  A scale outside of this range would require regenerating the distance fields
    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = kMinDFFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
        *textRatio = textSize / kSmallDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
        *textRatio = textSize / kMediumDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kMediumDFFontSize));
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = kLargeDFFontLimit;
        *textRatio = textSize / kLargeDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kLargeDFFontSize));
    }

    // Because there can be multiple runs in the blob, we want the overall maxMinScale, and
    // minMaxScale to make regeneration decisions.  Specifically, we want the maximum minimum scale
    // we can tolerate before we'd drop to a lower mip size, and the minimum maximum scale we can
    // tolerate before we'd have to move to a large mip size.  When we actually test these values
    // we look at the delta in scale between the new viewmatrix and the old viewmatrix, and test
    // against these values to decide if we can reuse or not(ie, will a given scale change our mip
    // level)
    SkASSERT(dfMaskScaleFloor <= scaledTextSize && scaledTextSize <= dfMaskScaleCeil);
    blob->fMaxMinScale = SkMaxScalar(dfMaskScaleFloor / scaledTextSize, blob->fMaxMinScale);
    blob->fMinMaxScale = SkMinScalar(dfMaskScaleCeil / scaledTextSize, blob->fMinMaxScale);

    skPaint->setLCDRenderText(false);
    skPaint->setAutohinted(false);
    skPaint->setHinting(SkPaint::kNormal_Hinting);
    skPaint->setSubpixelText(true);
}

inline void GrAtlasTextContext::fallbackDrawPosText(GrAtlasTextBlob* blob,
                                                    int runIndex,
                                                    const GrClip& clip,
                                                    GrColor color,
                                                    const SkPaint& skPaint,
                                                    const SkMatrix& viewMatrix,
                                                    const SkTDArray<char>& fallbackTxt,
                                                    const SkTDArray<SkScalar>& fallbackPos,
                                                    int scalarsPerPosition,
                                                    const SkPoint& offset) {
    SkASSERT(fallbackTxt.count());
    blob->setHasBitmap();
    Run& run = blob->fRuns[runIndex];
    // Push back a new subrun to fill and set the override descriptor
    run.push_back();
    run.fOverrideDescriptor.reset(new SkAutoDescriptor);
    skPaint.getScalerContextDescriptor(run.fOverrideDescriptor,
                                       fSurfaceProps, &viewMatrix, false);
    SkGlyphCache* cache = SkGlyphCache::DetachCache(run.fTypeface,
                                                    run.fOverrideDescriptor->getDesc());
    this->internalDrawBMPPosText(blob, runIndex, cache, skPaint, color, viewMatrix,
                                 fallbackTxt.begin(), fallbackTxt.count(),
                                 fallbackPos.begin(), scalarsPerPosition, offset);
    SkGlyphCache::AttachCache(cache);
}

inline GrAtlasTextBlob*
GrAtlasTextContext::setupDFBlob(int glyphCount, const SkPaint& origPaint,
                                const SkMatrix& viewMatrix, SkPaint* dfPaint,
                                SkScalar* textRatio) {
    GrAtlasTextBlob* blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBatch::kGrayTextVASize);

    *dfPaint = origPaint;
    this->initDistanceFieldPaint(blob, dfPaint, textRatio, viewMatrix);
    blob->fViewMatrix = viewMatrix;
    Run& run = blob->fRuns[0];
    PerSubRunInfo& subRun = run.fSubRunInfo.back();
    subRun.fUseLCDText = origPaint.isLCDRenderText();
    subRun.fDrawAsDistanceFields = true;

    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::createDrawTextBlob(const GrClip& clip,
                                       const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob;
    if (this->canDrawAsDistanceFields(skPaint, viewMatrix)) {
        SkPaint dfPaint;
        SkScalar textRatio;
        blob = this->setupDFBlob(glyphCount, skPaint, viewMatrix, &dfPaint, &textRatio);

        SkTDArray<char> fallbackTxt;
        SkTDArray<SkScalar> fallbackPos;
        SkPoint offset;
        this->internalDrawDFText(blob, 0, dfPaint, paint.getColor(), viewMatrix, text,
                                 byteLength, x, y, textRatio, &fallbackTxt, &fallbackPos,
                                 &offset, skPaint);
        if (fallbackTxt.count()) {
            this->fallbackDrawPosText(blob, 0, clip, paint.getColor(), skPaint, viewMatrix,
                                      fallbackTxt, fallbackPos, 2, offset);
        }
    } else {
        blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBatch::kGrayTextVASize);
        blob->fViewMatrix = viewMatrix;

        SkGlyphCache* cache = this->setupCache(&blob->fRuns[0], skPaint, &viewMatrix, false);
        this->internalDrawBMPText(blob, 0, cache, skPaint, paint.getColor(), viewMatrix, text,
                                  byteLength, x, y);
        SkGlyphCache::AttachCache(cache);
    }
    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::createDrawPosTextBlob(const GrClip& clip,
                                          const GrPaint& paint, const SkPaint& skPaint,
                                          const SkMatrix& viewMatrix,
                                          const char text[], size_t byteLength,
                                          const SkScalar pos[], int scalarsPerPosition,
                                          const SkPoint& offset, const SkIRect& regionClipBounds) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob;
    if (this->canDrawAsDistanceFields(skPaint, viewMatrix)) {
        SkPaint dfPaint;
        SkScalar textRatio;
        blob = this->setupDFBlob(glyphCount, skPaint, viewMatrix, &dfPaint, &textRatio);

        SkTDArray<char> fallbackTxt;
        SkTDArray<SkScalar> fallbackPos;
        this->internalDrawDFPosText(blob, 0, dfPaint, paint.getColor(), viewMatrix, text,
                                    byteLength, pos, scalarsPerPosition, offset,
                                    textRatio, &fallbackTxt, &fallbackPos);
        if (fallbackTxt.count()) {
            this->fallbackDrawPosText(blob, 0, clip, paint.getColor(), skPaint, viewMatrix,
                                      fallbackTxt, fallbackPos, scalarsPerPosition, offset);
        }
    } else {
        blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBatch::kGrayTextVASize);
        blob->fViewMatrix = viewMatrix;
        SkGlyphCache* cache = this->setupCache(&blob->fRuns[0], skPaint, &viewMatrix, false);
        this->internalDrawBMPPosText(blob, 0, cache, skPaint, paint.getColor(), viewMatrix, text,
                                     byteLength, pos, scalarsPerPosition, offset);
        SkGlyphCache::AttachCache(cache);
    }
    return blob;
}

void GrAtlasTextContext::onDrawText(GrDrawContext* dc, GrRenderTarget* rt,
                                    const GrClip& clip,
                                    const GrPaint& paint, const SkPaint& skPaint,
                                    const SkMatrix& viewMatrix,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawTextBlob(clip, paint, skPaint, viewMatrix,
                                 text, byteLength, x, y, regionClipBounds));
    this->flush(blob, dc, rt, skPaint, paint, clip, regionClipBounds);
}

void GrAtlasTextContext::onDrawPosText(GrDrawContext* dc, GrRenderTarget* rt,
                                       const GrClip& clip,
                                       const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       const SkScalar pos[], int scalarsPerPosition,
                                       const SkPoint& offset, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawPosTextBlob(clip, paint, skPaint, viewMatrix,
                                    text, byteLength,
                                    pos, scalarsPerPosition,
                                    offset, regionClipBounds));

    this->flush(blob, dc, rt, skPaint, paint, clip, regionClipBounds);
}

void GrAtlasTextContext::internalDrawBMPText(GrAtlasTextBlob* blob, int runIndex,
                                             SkGlyphCache* cache, const SkPaint& skPaint,
                                             GrColor color,
                                             const SkMatrix& viewMatrix,
                                             const char text[], size_t byteLength,
                                             SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    fCurrStrike = nullptr;

    // Get GrFontScaler from cache
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    SkFindAndPlaceGlyph::ProcessText(
        skPaint.getTextEncoding(), text, byteLength,
        {x, y}, viewMatrix, skPaint.getTextAlign(),
        cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
            position += rounding;
            this->bmpAppendGlyph(
                blob, runIndex, glyph,
                SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                color, fontScaler);
        }
    );
}

void GrAtlasTextContext::internalDrawBMPPosText(GrAtlasTextBlob* blob, int runIndex,
                                                SkGlyphCache* cache, const SkPaint& skPaint,
                                                GrColor color,
                                                const SkMatrix& viewMatrix,
                                                const char text[], size_t byteLength,
                                                const SkScalar pos[], int scalarsPerPosition,
                                                const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    fCurrStrike = nullptr;

    // Get GrFontScaler from cache
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    SkFindAndPlaceGlyph::ProcessPosText(
        skPaint.getTextEncoding(), text, byteLength,
        offset, viewMatrix, pos, scalarsPerPosition,
        skPaint.getTextAlign(), cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
            position += rounding;
            this->bmpAppendGlyph(
                blob, runIndex, glyph,
                SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                color, fontScaler);
        }
    );
}

void GrAtlasTextContext::internalDrawDFText(GrAtlasTextBlob* blob, int runIndex,
                                            const SkPaint& skPaint, GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const char text[], size_t byteLength,
                                            SkScalar x, SkScalar y,
                                            SkScalar textRatio,
                                            SkTDArray<char>* fallbackTxt,
                                            SkTDArray<SkScalar>* fallbackPos,
                                            SkPoint* offset,
                                            const SkPaint& origPaint) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    SkDrawCacheProc glyphCacheProc = origPaint.getDrawCacheProc();
    SkAutoDescriptor desc;
    origPaint.getScalerContextDescriptor(&desc, fSurfaceProps, nullptr, true);
    SkGlyphCache* origPaintCache = SkGlyphCache::DetachCache(origPaint.getTypeface(),
                                                             desc.getDesc());

    SkTArray<SkScalar> positions;

    const char* textPtr = text;
    SkFixed stopX = 0;
    SkFixed stopY = 0;
    SkFixed origin = 0;
    switch (origPaint.getTextAlign()) {
        case SkPaint::kRight_Align: origin = SK_Fixed1; break;
        case SkPaint::kCenter_Align: origin = SK_FixedHalf; break;
        case SkPaint::kLeft_Align: origin = 0; break;
    }

    SkAutoKern autokern;
    const char* stop = text + byteLength;
    while (textPtr < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(origPaintCache, &textPtr, 0, 0);

        SkFixed width = glyph.fAdvanceX + autokern.adjust(glyph);
        positions.push_back(SkFixedToScalar(stopX + SkFixedMul(origin, width)));

        SkFixed height = glyph.fAdvanceY;
        positions.push_back(SkFixedToScalar(stopY + SkFixedMul(origin, height)));

        stopX += width;
        stopY += height;
    }
    SkASSERT(textPtr == stop);

    SkGlyphCache::AttachCache(origPaintCache);

    // now adjust starting point depending on alignment
    SkScalar alignX = SkFixedToScalar(stopX);
    SkScalar alignY = SkFixedToScalar(stopY);
    if (origPaint.getTextAlign() == SkPaint::kCenter_Align) {
        alignX = SkScalarHalf(alignX);
        alignY = SkScalarHalf(alignY);
    } else if (origPaint.getTextAlign() == SkPaint::kLeft_Align) {
        alignX = 0;
        alignY = 0;
    }
    x -= alignX;
    y -= alignY;
    *offset = SkPoint::Make(x, y);

    this->internalDrawDFPosText(blob, runIndex, skPaint, color, viewMatrix, text, byteLength,
                                positions.begin(), 2, *offset, textRatio, fallbackTxt,
                                fallbackPos);
}

void GrAtlasTextContext::internalDrawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                                               const SkPaint& skPaint, GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[], int scalarsPerPosition,
                                               const SkPoint& offset,
                                               SkScalar textRatio,
                                               SkTDArray<char>* fallbackTxt,
                                               SkTDArray<SkScalar>* fallbackPos) {

    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    fCurrStrike = nullptr;

    SkDrawCacheProc glyphCacheProc = skPaint.getDrawCacheProc();
    SkGlyphCache* cache = this->setupCache(&blob->fRuns[runIndex], skPaint, nullptr, true);
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    const char* stop = text + byteLength;

    if (SkPaint::kLeft_Align == skPaint.getTextAlign()) {
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                if (!this->dfAppendGlyph(blob,
                                         runIndex,
                                         glyph,
                                         x, y, color, fontScaler,
                                         textRatio, viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt->append(SkToInt(text-lastText), lastText);
                    *fallbackPos->append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos->append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    } else {
        SkScalar alignMul = SkPaint::kCenter_Align == skPaint.getTextAlign() ? SK_ScalarHalf
                                                                             : SK_Scalar1;
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                SkScalar advanceX = SkFixedToScalar(glyph.fAdvanceX) * alignMul * textRatio;
                SkScalar advanceY = SkFixedToScalar(glyph.fAdvanceY) * alignMul * textRatio;

                if (!this->dfAppendGlyph(blob,
                                         runIndex,
                                         glyph,
                                         x - advanceX, y - advanceY, color,
                                         fontScaler,
                                         textRatio,
                                         viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt->append(SkToInt(text-lastText), lastText);
                    *fallbackPos->append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos->append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    }

    SkGlyphCache::AttachCache(cache);
}

void GrAtlasTextContext::bmpAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                        const SkGlyph& skGlyph,
                                        int vx, int vy, GrColor color, GrFontScaler* scaler) {
    Run& run = blob->fRuns[runIndex];
    if (!fCurrStrike) {
        fCurrStrike = fContext->getBatchFontCache()->getStrike(scaler);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kCoverage_MaskStyle);
    GrGlyph* glyph = fCurrStrike->getGlyph(skGlyph, id, scaler);
    if (!glyph) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    // If the glyph is too large we fall back to paths
    if (glyph->fTooLargeForAtlas) {
        this->appendGlyphPath(blob, glyph, scaler, skGlyph, SkIntToScalar(vx), SkIntToScalar(vy));
        return;
    }

    GrMaskFormat format = glyph->fMaskFormat;

    PerSubRunInfo* subRun = &run.fSubRunInfo.back();
    if (run.fInitialized && subRun->fMaskFormat != format) {
        subRun = &run.push_back();
        subRun->fStrike.reset(SkRef(fCurrStrike));
    } else if (!run.fInitialized) {
        subRun->fStrike.reset(SkRef(fCurrStrike));
    }

    run.fInitialized = true;

    size_t vertexStride = GrAtlasTextBatch::GetVertexStride(format);

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);
    subRun->fMaskFormat = format;
    this->appendGlyphCommon(blob, &run, subRun, r, color, vertexStride, kA8_GrMaskFormat == format,
                            glyph);
}

bool GrAtlasTextContext::dfAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                       const SkGlyph& skGlyph,
                                       SkScalar sx, SkScalar sy, GrColor color,
                                       GrFontScaler* scaler,
                                       SkScalar textRatio, const SkMatrix& viewMatrix) {
    Run& run = blob->fRuns[runIndex];
    if (!fCurrStrike) {
        fCurrStrike = fContext->getBatchFontCache()->getStrike(scaler);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kDistance_MaskStyle);
    GrGlyph* glyph = fCurrStrike->getGlyph(skGlyph, id, scaler);
    if (!glyph) {
        return true;
    }

    // fallback to color glyph support
    if (kA8_GrMaskFormat != glyph->fMaskFormat) {
        return false;
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2 * SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2 * SK_DistanceFieldInset);

    SkScalar scale = textRatio;
    dx *= scale;
    dy *= scale;
    width *= scale;
    height *= scale;
    sx += dx;
    sy += dy;
    SkRect glyphRect = SkRect::MakeXYWH(sx, sy, width, height);

    // TODO combine with the above
    // If the glyph is too large we fall back to paths
    if (glyph->fTooLargeForAtlas) {
        this->appendGlyphPath(blob, glyph, scaler, skGlyph, sx - dx, sy - dy, scale, true);
        return true;
    }

    PerSubRunInfo* subRun = &run.fSubRunInfo.back();
    if (!run.fInitialized) {
        subRun->fStrike.reset(SkRef(fCurrStrike));
    }
    run.fInitialized = true;
    SkASSERT(glyph->fMaskFormat == kA8_GrMaskFormat);
    subRun->fMaskFormat = kA8_GrMaskFormat;

    size_t vertexStride = GrAtlasTextBatch::GetVertexStrideDf(kA8_GrMaskFormat,
                                                              subRun->fUseLCDText);

    bool useColorVerts = !subRun->fUseLCDText;
    this->appendGlyphCommon(blob, &run, subRun, glyphRect, color, vertexStride, useColorVerts,
                            glyph);
    return true;
}

inline void GrAtlasTextContext::appendGlyphPath(GrAtlasTextBlob* blob, GrGlyph* glyph,
                                                GrFontScaler* scaler, const SkGlyph& skGlyph,
                                                SkScalar x, SkScalar y, SkScalar scale,
                                                bool applyVM) {
    if (nullptr == glyph->fPath) {
        const SkPath* glyphPath = scaler->getGlyphPath(skGlyph);
        if (!glyphPath) {
            return;
        }

        glyph->fPath = new SkPath(*glyphPath);
    }
    blob->fBigGlyphs.push_back(GrAtlasTextBlob::BigGlyph(*glyph->fPath, x, y, scale, applyVM));
}

inline void GrAtlasTextContext::appendGlyphCommon(GrAtlasTextBlob* blob, Run* run,
                                                  Run::SubRunInfo* subRun,
                                                  const SkRect& positions, GrColor color,
                                                  size_t vertexStride, bool useVertexColor,
                                                  GrGlyph* glyph) {
    blob->fGlyphs[subRun->fGlyphEndIndex] = glyph;
    run->fVertexBounds.joinNonEmptyArg(positions);
    run->fColor = color;

    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices + subRun->fVertexEndIndex);

    if (useVertexColor) {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    } else {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
    }

    subRun->fGlyphEndIndex++;
    subRun->fVertexEndIndex += vertexStride * GrAtlasTextBatch::kVerticesPerGlyph;
}

void GrAtlasTextContext::flushRunAsPaths(GrDrawContext* dc,
                                         const SkTextBlobRunIterator& it,
                                         const GrClip& clip, const SkPaint& skPaint,
                                         SkDrawFilter* drawFilter, const SkMatrix& viewMatrix,
                                         const SkIRect& clipBounds, SkScalar x, SkScalar y) {
    SkPaint runPaint = skPaint;

    size_t textLen = it.glyphCount() * sizeof(uint16_t);
    const SkPoint& offset = it.offset();

    it.applyFontToPaint(&runPaint);

    if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
        return;
    }

    runPaint.setFlags(FilterTextFlags(fSurfaceProps, runPaint));

    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            this->drawTextAsPath(dc, clip, runPaint, viewMatrix,
                                 (const char *)it.glyphs(),
                                 textLen, x + offset.x(), y + offset.y(), clipBounds);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            this->drawPosTextAsPath(dc, clip, runPaint, viewMatrix,
                                    (const char*)it.glyphs(),
                                    textLen, it.pos(), 1, SkPoint::Make(x, y + offset.y()),
                                    clipBounds);
            break;
        case SkTextBlob::kFull_Positioning:
            this->drawPosTextAsPath(dc, clip, runPaint, viewMatrix,
                                    (const char*)it.glyphs(),
                                    textLen, it.pos(), 2, SkPoint::Make(x, y), clipBounds);
            break;
    }
}

inline GrDrawBatch*
GrAtlasTextContext::createBatch(GrAtlasTextBlob* cacheBlob, const PerSubRunInfo& info,
                                int glyphCount, int run, int subRun,
                                GrColor color, SkScalar transX, SkScalar transY,
                                const SkPaint& skPaint) {
    GrMaskFormat format = info.fMaskFormat;
    GrColor subRunColor;
    if (kARGB_GrMaskFormat == format) {
        uint8_t paintAlpha = skPaint.getAlpha();
        subRunColor = SkColorSetARGB(paintAlpha, paintAlpha, paintAlpha, paintAlpha);
    } else {
        subRunColor = color;
    }

    GrAtlasTextBatch* batch;
    if (info.fDrawAsDistanceFields) {
        SkColor filteredColor;
        SkColorFilter* colorFilter = skPaint.getColorFilter();
        if (colorFilter) {
            filteredColor = colorFilter->filterColor(skPaint.getColor());
        } else {
            filteredColor = skPaint.getColor();
        }
        bool useBGR = SkPixelGeometryIsBGR(fSurfaceProps.pixelGeometry());
        batch = GrAtlasTextBatch::CreateDistanceField(glyphCount, fContext->getBatchFontCache(),
                                                      fDistanceAdjustTable, filteredColor,
                                                      info.fUseLCDText, useBGR);
    } else {
        batch = GrAtlasTextBatch::CreateBitmap(format, glyphCount, fContext->getBatchFontCache());
    }
    GrAtlasTextBatch::Geometry& geometry = batch->geometry();
    geometry.fBlob = SkRef(cacheBlob);
    geometry.fRun = run;
    geometry.fSubRun = subRun;
    geometry.fColor = subRunColor;
    geometry.fTransX = transX;
    geometry.fTransY = transY;
    batch->init();

    return batch;
}

inline void GrAtlasTextContext::flushRun(GrDrawContext* dc, GrPipelineBuilder* pipelineBuilder,
                                         GrAtlasTextBlob* cacheBlob, int run, GrColor color,
                                         SkScalar transX, SkScalar transY,
                                         const SkPaint& skPaint) {
    for (int subRun = 0; subRun < cacheBlob->fRuns[run].fSubRunInfo.count(); subRun++) {
        const PerSubRunInfo& info = cacheBlob->fRuns[run].fSubRunInfo[subRun];
        int glyphCount = info.fGlyphEndIndex - info.fGlyphStartIndex;
        if (0 == glyphCount) {
            continue;
        }

        SkAutoTUnref<GrDrawBatch> batch(this->createBatch(cacheBlob, info, glyphCount, run,
                                                          subRun, color, transX, transY,
                                                          skPaint));
        dc->drawBatch(pipelineBuilder, batch);
    }
}

inline void GrAtlasTextContext::flushBigGlyphs(GrAtlasTextBlob* cacheBlob,
                                               GrDrawContext* dc,
                                               const GrClip& clip, const SkPaint& skPaint,
                                               SkScalar transX, SkScalar transY,
                                               const SkIRect& clipBounds) {
    if (!cacheBlob->fBigGlyphs.count()) {
        return;
    }

    for (int i = 0; i < cacheBlob->fBigGlyphs.count(); i++) {
        GrAtlasTextBlob::BigGlyph& bigGlyph = cacheBlob->fBigGlyphs[i];
        bigGlyph.fVx += transX;
        bigGlyph.fVy += transY;
        SkMatrix ctm;
        ctm.setScale(bigGlyph.fScale, bigGlyph.fScale);
        ctm.postTranslate(bigGlyph.fVx, bigGlyph.fVy);
        if (bigGlyph.fApplyVM) {
            ctm.postConcat(cacheBlob->fViewMatrix);
        }

        GrBlurUtils::drawPathWithMaskFilter(fContext, dc, clip, bigGlyph.fPath,
                                            skPaint, ctm, nullptr, clipBounds, false);
    }
}

void GrAtlasTextContext::flush(const SkTextBlob* blob,
                               GrAtlasTextBlob* cacheBlob,
                               GrDrawContext* dc,
                               GrRenderTarget* rt,
                               const SkPaint& skPaint,
                               const GrPaint& grPaint,
                               SkDrawFilter* drawFilter,
                               const GrClip& clip,
                               const SkMatrix& viewMatrix,
                               const SkIRect& clipBounds,
                               SkScalar x, SkScalar y,
                               SkScalar transX, SkScalar transY) {
    // We loop through the runs of the blob, flushing each.  If any run is too large, then we flush
    // it as paths
    GrPipelineBuilder pipelineBuilder(grPaint, rt, clip);

    GrColor color = grPaint.getColor();

    SkTextBlobRunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        if (cacheBlob->fRuns[run].fDrawAsPaths) {
            this->flushRunAsPaths(dc, it, clip, skPaint,
                                  drawFilter, viewMatrix, clipBounds, x, y);
            continue;
        }
        cacheBlob->fRuns[run].fVertexBounds.offset(transX, transY);
        this->flushRun(dc, &pipelineBuilder, cacheBlob, run, color,
                       transX, transY, skPaint);
    }

    // Now flush big glyphs
    this->flushBigGlyphs(cacheBlob, dc, clip, skPaint, transX, transY, clipBounds);
}

void GrAtlasTextContext::flush(GrAtlasTextBlob* cacheBlob,
                               GrDrawContext* dc,
                               GrRenderTarget* rt,
                               const SkPaint& skPaint,
                               const GrPaint& grPaint,
                               const GrClip& clip,
                               const SkIRect& clipBounds) {
    GrPipelineBuilder pipelineBuilder(grPaint, rt, clip);

    GrColor color = grPaint.getColor();
    for (int run = 0; run < cacheBlob->fRunCount; run++) {
        this->flushRun(dc, &pipelineBuilder, cacheBlob, run, color, 0, 0, skPaint);
    }

    // Now flush big glyphs
    this->flushBigGlyphs(cacheBlob, dc, clip, skPaint, 0, 0, clipBounds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(TextBlobBatch) {
    static uint32_t gContextID = SK_InvalidGenID;
    static GrAtlasTextContext* gTextContext = nullptr;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        delete gTextContext;

        // We don't yet test the fall back to paths in the GrTextContext base class.  This is mostly
        // because we don't really want to have a gpu device here.
        // We enable distance fields by twiddling a knob on the paint
        gTextContext = GrAtlasTextContext::Create(context, gSurfaceProps);
    }

    // Setup dummy SkPaint / GrPaint
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPaint skPaint;
    skPaint.setColor(color);
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, &grPaint)) {
        SkFAIL("couldn't convert paint\n");
    }

    const char* text = "The quick brown fox jumps over the lazy dog.";
    int textLen = (int)strlen(text);

    // Setup clip
    GrClip clip;
    SkIRect noClip = SkIRect::MakeLargest();

    // right now we don't handle textblobs, nor do we handle drawPosText.  Since we only
    // intend to test the batch with this unit test, that is okay.
    SkAutoTUnref<GrAtlasTextBlob> blob(
            gTextContext->createDrawTextBlob(clip, grPaint, skPaint, viewMatrix, text,
                                             static_cast<size_t>(textLen), 0, 0, noClip));

    SkScalar transX = static_cast<SkScalar>(random->nextU());
    SkScalar transY = static_cast<SkScalar>(random->nextU());
    const GrAtlasTextBlob::Run::SubRunInfo& info = blob->fRuns[0].fSubRunInfo[0];
    return gTextContext->createBatch(blob, info, textLen, 0, 0, color, transX, transY, skPaint);
}

#endif
