/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRunPainter.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/gpu/text/GrTextContext.h"
#endif

#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeForGPU.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTraceEvent.h"

#include <climits>

// -- SkGlyphRunListPainter ------------------------------------------------------------------------
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkScalerContextFlags flags,
                                             SkStrikeForGPUCacheInterface* strikeCache)
        : fDeviceProps{props}
        ,  fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        ,  fColorType{colorType}, fScalerContextFlags{flags}
        ,  fStrikeCache{strikeCache} {}

// TODO: unify with code in GrTextContext.cpp
static SkScalerContextFlags compute_scaler_context_flags(const SkColorSpace* cs) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (cs && cs->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkColorSpace* cs,
                                             SkStrikeForGPUCacheInterface* strikeCache)
        : SkGlyphRunListPainter(props, colorType, compute_scaler_context_flags(cs), strikeCache) {}

#if SK_SUPPORT_GPU
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props, const GrColorInfo& csi)
        : SkGlyphRunListPainter(props,
                                kUnknown_SkColorType,
                                compute_scaler_context_flags(csi.colorSpace()),
                                SkStrikeCache::GlobalStrikeCache()) {}

SkGlyphRunListPainter::SkGlyphRunListPainter(const GrRenderTargetContext& rtc)
        : SkGlyphRunListPainter{rtc.surfaceProps(), rtc.colorInfo()} {}

#endif

void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
        const BitmapDevicePainter* bitmapDevice) {
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    // TODO: fStrikeCache is only used for GPU, and some compilers complain about it during the no
    //  gpu build. Remove when SkGlyphRunListPainter is split into GPU and CPU version.
    (void)fStrikeCache;

    const SkPaint& runPaint = glyphRunList.paint();
    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && runPaint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint drawOrigin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        fRejects.setSource(glyphRun.source());

        if (SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, deviceMatrix)) {

            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                    runFont, runPaint, props, fScalerContextFlags);

            auto strike = strikeSpec.findOrCreateStrike();

            fDrawable.startSource(fRejects.source(), drawOrigin);
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            SkPaint pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(&fDrawable, strikeSpec.strikeToSourceRatio(), pathPaint);
        }
        if (!fRejects.source().empty()) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix);

            auto strike = strikeSpec.findOrCreateStrike();

            fDrawable.startDevice(
                    fRejects.source(), drawOrigin, deviceMatrix, strike->roundingSpec());
            strike->prepareForDrawingMasksCPU(&fDrawable);
            bitmapDevice->paintMasks(&fDrawable, runPaint);
        }

        // TODO: have the mask stage above reject the glyphs that are too big, and handle the
        //  rejects in a more sophisticated stage.
    }
}

#if SK_SUPPORT_GPU
void SkGlyphRunListPainter::processGlyphRunList(const SkGlyphRunList& glyphRunList,
                                                const SkMatrix& drawMatrix,
                                                const SkSurfaceProps& props,
                                                bool contextSupportsDistanceFieldText,
                                                const GrTextContext::Options& options,
                                                SkGlyphRunPainterInterface* process) {

    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    for (const auto& glyphRun : glyphRunList) {
        fRejects.setSource(glyphRun.source());
        const SkFont& runFont = glyphRun.font();


        bool useSDFT = GrTextContext::CanDrawAsDistanceFields(
                runPaint, runFont, drawMatrix, props, contextSupportsDistanceFieldText, options);

        bool usePaths =
                useSDFT ? false : SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, drawMatrix);

        if (useSDFT) {
            // Process SDFT - This should be the .009% case.
            SkScalar minScale, maxScale;
            SkStrikeSpec strikeSpec;
            std::tie(strikeSpec, minScale, maxScale) =
                    SkStrikeSpec::MakeSDFT(runFont, runPaint, fDeviceProps, drawMatrix, options);

            if (!strikeSpec.isEmpty()) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startSource(fRejects.source(), origin);
                strike->prepareForSDFTDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();

                if (process) {
                    // processSourceSDFT must be called even if there are no glyphs to make sure
                    // runs are set correctly.
                    process->processSourceSDFT(
                            fDrawable.drawable(), strikeSpec, runFont, minScale, maxScale);
                }
            }
        }

        if (!usePaths && !fRejects.source().empty()) {
            // Process masks including ARGB - this should be the 99.99% case.

            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, fDeviceProps, fScalerContextFlags, drawMatrix);

            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startDevice(fRejects.source(), origin, drawMatrix, strike->roundingSpec());
            strike->prepareForMaskDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            if (process) {
                // processDeviceMasks must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processDeviceMasks(fDrawable.drawable(), strikeSpec);
            }
        }

        // Glyphs are generated in different scales relative to the source space. Masks are drawn
        // in device space, and SDFT and Paths are draw in a fixed constant space. The
        // maxDimensionInSourceSpace is used to calculate the factor from strike space to source
        // space.
        SkScalar maxDimensionInSourceSpace = 0.0;
        if (!fRejects.source().empty()) {
            // Path case - handle big things without color and that have a path.
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                    runFont, runPaint, fDeviceProps, fScalerContextFlags);

            if (!strikeSpec.isEmpty()) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startPaths(fRejects.source());
                strike->prepareForPathDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();
                maxDimensionInSourceSpace =
                        fRejects.rejectedMaxDimension() * strikeSpec.strikeToSourceRatio();

                if (process) {
                    // processSourcePaths must be called even if there are no glyphs to make sure
                    // runs are set correctly.
                    process->processSourcePaths(fDrawable.drawable(), runFont, strikeSpec);
                }
            }
        }

        if (!fRejects.source().empty() && maxDimensionInSourceSpace != 0) {
            // Draw of last resort. Scale the bitmap to the screen.
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeSourceFallback(
                    runFont, runPaint, fDeviceProps,
                    fScalerContextFlags, maxDimensionInSourceSpace);

            if (!strikeSpec.isEmpty()) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startSource(fRejects.source(), origin);
                strike->prepareForMaskDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();
                SkASSERT(fRejects.source().empty());

                if (process) {
                    process->processSourceMasks(fDrawable.drawable(), strikeSpec);
                }
            }
        }
    }  // For all glyph runs
}
#endif  // SK_SUPPORT_GPU

auto SkGlyphRunListPainter::ensureBuffers(const SkGlyphRunList& glyphRunList) -> ScopedBuffers {
    size_t size = 0;
    for (const SkGlyphRun& run : glyphRunList) {
        size = std::max(run.runSize(), size);
    }
    return ScopedBuffers(this, size);
}

#if SK_SUPPORT_GPU
// -- GrTextContext --------------------------------------------------------------------------------
SkPMColor4f generate_filtered_color(const SkPaint& paint, const GrColorInfo& colorInfo) {
    SkColor4f filteredColor = paint.getColor4f();
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        filteredColor = xform->apply(filteredColor);
    }
    if (paint.getColorFilter() != nullptr) {
        filteredColor = paint.getColorFilter()->filterColor4f(filteredColor, colorInfo.colorSpace(),
                                                              colorInfo.colorSpace());
    }
    return filteredColor.premul();
}

void GrTextContext::drawGlyphRunList(
        GrRecordingContext* context, GrTextTarget* target, const GrClip& clip,
        const SkMatrix& drawMatrix, const SkSurfaceProps& props,
        const SkGlyphRunList& glyphRunList) {
    auto contextPriv = context->priv();
    // If we have been abandoned, then don't draw
    if (contextPriv.abandoned()) {
        return;
    }
    auto grStrikeCache = contextPriv.getGrStrikeCache();
    GrTextBlobCache* textBlobCache = contextPriv.getTextBlobCache();

    // Get the first paint to use as the key paint.
    const SkPaint& blobPaint = glyphRunList.paint();

    const GrColorInfo& colorInfo = target->colorInfo();
    // This is the color the op will use to draw.
    SkPMColor4f drawingColor = generate_filtered_color(blobPaint, colorInfo);
    // When creating the a new blob, use the GrColor calculated from the drawingColor.
    GrColor initialVertexColor = drawingColor.toBytes_RGBA();

    SkPoint drawOrigin = glyphRunList.origin();

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = blobPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() && !(blobPaint.getPathEffect() ||
                                                (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(colorInfo);

    sk_sp<GrTextBlob> cachedBlob;
    GrTextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? props.pixelGeometry() :
                                        kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                 ComputeCanonicalColor(blobPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = blobPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = scalerContextFlags;
        cachedBlob = textBlobCache->find(key);
    }

    bool forceW = fOptions.fDistanceFieldVerticesAlwaysHaveW;
    bool supportsSDFT = context->priv().caps()->shaderCaps()->supportsDistanceFieldText();
    SkGlyphRunListPainter* painter = target->glyphPainter();
    if (cachedBlob) {
        if (cachedBlob->mustRegenerate(blobPaint, glyphRunList.anyRunsSubpixelPositioned(),
                                       blurRec, drawMatrix, drawOrigin)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cachedBlob.get());
            cachedBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, grStrikeCache, key, blurRec, drawMatrix,
                    initialVertexColor, forceW);

            painter->processGlyphRunList(
                    glyphRunList, drawMatrix, props, supportsSDFT, fOptions, cachedBlob.get());
        } else {
            textBlobCache->makeMRU(cachedBlob.get());
        }
    } else {
        if (canCache) {
            cachedBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, grStrikeCache, key, blurRec, drawMatrix,
                    initialVertexColor, forceW);
        } else {
            cachedBlob = textBlobCache->makeBlob(
                    glyphRunList, grStrikeCache, drawMatrix, initialVertexColor, forceW);
        }
        painter->processGlyphRunList(
                glyphRunList, drawMatrix, props, supportsSDFT, fOptions, cachedBlob.get());
    }

    cachedBlob->flush(target, props, blobPaint, drawingColor, clip, drawMatrix, drawOrigin);
}

#if GR_TEST_UTILS

#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"

std::unique_ptr<GrDrawOp> GrTextContext::createOp_TestingOnly(GrRecordingContext* context,
                                                              GrTextContext* textContext,
                                                              GrRenderTargetContext* rtc,
                                                              const SkPaint& skPaint,
                                                              const SkFont& font,
                                                              const SkMatrix& drawMatrix,
                                                              const char* text,
                                                              int x,
                                                              int y) {
    auto direct = context->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    auto strikeCache = direct->priv().getGrStrikeCache();

    static SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    size_t textLen = (int)strlen(text);

    SkPMColor4f filteredColor = generate_filtered_color(skPaint, rtc->colorInfo());
    GrColor color = filteredColor.toBytes_RGBA();

    auto drawOrigin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawTextUTF8(skPaint, font, text, textLen, drawOrigin);

    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = direct->priv().getTextBlobCache()->makeBlob(
                glyphRunList, strikeCache, drawMatrix, color, false);
        SkGlyphRunListPainter* painter = rtc->textTarget()->glyphPainter();
        painter->processGlyphRunList(
                glyphRunList, drawMatrix, surfaceProps,
                context->priv().caps()->shaderCaps()->supportsDistanceFieldText(),
                textContext->fOptions, blob.get());
    }

    return blob->test_makeOp(textLen, drawMatrix, drawOrigin, skPaint, filteredColor, surfaceProps,
                             rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU

SkGlyphRunListPainter::ScopedBuffers::ScopedBuffers(SkGlyphRunListPainter* painter, size_t size)
        : fPainter{painter} {
    fPainter->fDrawable.ensureSize(size);
}

SkGlyphRunListPainter::ScopedBuffers::~ScopedBuffers() {
    fPainter->fDrawable.reset();
    fPainter->fRejects.reset();
}

SkVector SkGlyphPositionRoundingSpec::HalfAxisSampleFreq(
        bool isSubpixel, SkAxisAlignment axisAlignment) {
    if (!isSubpixel) {
        return {SK_ScalarHalf, SK_ScalarHalf};
    } else {
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {SkPackedGlyphID::kSubpixelRound, SK_ScalarHalf};
            case kY_SkAxisAlignment:
                return {SK_ScalarHalf, SkPackedGlyphID::kSubpixelRound};
            case kNone_SkAxisAlignment:
                return {SkPackedGlyphID::kSubpixelRound, SkPackedGlyphID::kSubpixelRound};
        }
    }

    // Some compilers need this.
    return {0, 0};
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionMask(
        bool isSubpixel, SkAxisAlignment axisAlignment) {
    return SkIPoint::Make((!isSubpixel || axisAlignment == kY_SkAxisAlignment) ? 0 : ~0,
                          (!isSubpixel || axisAlignment == kX_SkAxisAlignment) ? 0 : ~0);
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionFieldMask(bool isSubpixel,
                                                              SkAxisAlignment axisAlignment) {
    SkIPoint ignoreMask = IgnorePositionMask(isSubpixel, axisAlignment);
    SkIPoint answer{ignoreMask.x() & SkPackedGlyphID::kXYFieldMask.x(),
                    ignoreMask.y() & SkPackedGlyphID::kXYFieldMask.y()};
    return answer;
}

SkGlyphPositionRoundingSpec::SkGlyphPositionRoundingSpec(
        bool isSubpixel,SkAxisAlignment axisAlignment)
    : halfAxisSampleFreq{HalfAxisSampleFreq(isSubpixel, axisAlignment)}
    , ignorePositionMask{IgnorePositionMask(isSubpixel, axisAlignment)}
    , ignorePositionFieldMask {IgnorePositionFieldMask(isSubpixel, axisAlignment)}{ }
