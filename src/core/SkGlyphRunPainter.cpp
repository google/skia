/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRunPainter.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrSDFTOptions.h"
#include "src/gpu/text/GrTextBlobCache.h"
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

// TODO: unify with code in GrSDFTOptions.cpp
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

SkGlyphRunListPainter::SkGlyphRunListPainter(const GrSurfaceDrawContext& rtc)
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

            fDrawable.startSource(fRejects.source());
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            SkPaint pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(
                    &fDrawable, strikeSpec.strikeToSourceRatio(), drawOrigin, pathPaint);
        }
        if (!fRejects.source().empty()) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix);

            auto strike = strikeSpec.findOrCreateStrike();

            fDrawable.startBitmapDevice(
                    fRejects.source(), drawOrigin, deviceMatrix, strike->roundingSpec());
            strike->prepareForDrawingMasksCPU(&fDrawable);
            bitmapDevice->paintMasks(&fDrawable, runPaint);
        }

        // TODO: have the mask stage above reject the glyphs that are too big, and handle the
        //  rejects in a more sophisticated stage.
    }
}

#if SK_SUPPORT_GPU
void SkGlyphRunListPainter::processGlyphRun(const SkGlyphRun& glyphRun,
                                            const SkMatrix& drawMatrix,
                                            SkPoint drawOrigin,
                                            const SkPaint& runPaint,
                                            const SkSurfaceProps& props,
                                            bool contextSupportsDistanceFieldText,
                                            const GrSDFTOptions& options,
                                            SkGlyphRunPainterInterface* process) {
    ScopedBuffers _ = this->ensureBuffers(glyphRun);
    fRejects.setSource(glyphRun.source());
    const SkFont& runFont = glyphRun.font();
    bool useSDFT = options.canDrawAsDistanceFields(
            runPaint, runFont, drawMatrix, props, contextSupportsDistanceFieldText);

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

            fDrawable.startSource(fRejects.source());
            strike->prepareForSDFTDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            if (process && !fDrawable.drawableIsEmpty()) {
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

        fDrawable.startGPUDevice(fRejects.source(), drawOrigin, drawMatrix, strike->roundingSpec());
        strike->prepareForMaskDrawing(&fDrawable, &fRejects);
        fRejects.flipRejectsToSource();

        if (process && !fDrawable.drawableIsEmpty()) {
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

            fDrawable.startSource(fRejects.source());
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();
            maxDimensionInSourceSpace =
                    fRejects.rejectedMaxDimension() * strikeSpec.strikeToSourceRatio();

            if (process && !fDrawable.drawableIsEmpty()) {
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

            fDrawable.startSource(fRejects.source());
            strike->prepareForMaskDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();
            SkASSERT(fRejects.source().empty());

            if (process && !fDrawable.drawableIsEmpty()) {
                process->processSourceMasks(fDrawable.drawable(), strikeSpec);
            }
        }
    }
}
#endif  // SK_SUPPORT_GPU

auto SkGlyphRunListPainter::ensureBuffers(const SkGlyphRunList& glyphRunList) -> ScopedBuffers {
    size_t size = 0;
    for (const SkGlyphRun& run : glyphRunList) {
        size = std::max(run.runSize(), size);
    }
    return ScopedBuffers(this, size);
}

auto SkGlyphRunListPainter::ensureBuffers(const SkGlyphRun& glyphRun) -> ScopedBuffers {
    return ScopedBuffers(this, glyphRun.runSize());
}

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
