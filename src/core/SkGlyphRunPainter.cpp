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
#include "src/gpu/SkGr.h"
#include "src/gpu/text/GrSDFTControl.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#endif // SK_SUPPORT_GPU

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

#include <cinttypes>
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

// TODO: unify with code in GrSDFTControl.cpp
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

SkGlyphRunListPainter::SkGlyphRunListPainter(const skgpu::v1::SurfaceDrawContext& sdc)
        : SkGlyphRunListPainter{sdc.surfaceProps(), sdc.colorInfo()} {}

#endif // SK_SUPPORT_GPU

void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkPaint& paint, const SkMatrix& deviceMatrix,
        const BitmapDevicePainter* bitmapDevice) {
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    // TODO: fStrikeCache is only used for GPU, and some compilers complain about it during the no
    //  gpu build. Remove when SkGlyphRunListPainter is split into GPU and CPU version.
    (void)fStrikeCache;

    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && paint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint drawOrigin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        fRejects.setSource(glyphRun.source());

        if (SkStrikeSpec::ShouldDrawAsPath(paint, runFont, deviceMatrix)) {

            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, paint, props, fScalerContextFlags);

            auto strike = strikeSpec.findOrCreateStrike();

            fDrawable.startSource(fRejects.source());
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            SkPaint pathPaint = paint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(
                    &fDrawable, strikeToSourceScale, drawOrigin, pathPaint);
        }
        if (!fRejects.source().empty() && !deviceMatrix.hasPerspective()) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, deviceMatrix);

            auto strike = strikeSpec.findOrCreateStrike();

            fDrawable.startBitmapDevice(
                    fRejects.source(), drawOrigin, deviceMatrix, strike->roundingSpec());
            strike->prepareForDrawingMasksCPU(&fDrawable);
            fRejects.flipRejectsToSource();
            bitmapDevice->paintMasks(&fDrawable, paint);
        }
        if (!fRejects.source().empty()) {
            SkMatrix runMatrix = deviceMatrix;
            runMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
            std::vector<SkPoint> sourcePositions;

            // Create a strike is source space to calculate scale information.
            SkStrikeSpec scaleStrikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, SkMatrix::I());
            SkBulkGlyphMetrics metrics{scaleStrikeSpec};

            auto glyphIDs = fRejects.source().get<0>();
            auto positions = fRejects.source().get<1>();
            SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);
            SkScalar maxScale = SK_ScalarMin;

            // Calculate the scale that makes the longest edge 1:1 with its side in the cache.
            for (auto [glyph, pos] : SkMakeZip(glyphs, positions)) {
                SkPoint corners[4];
                SkPoint srcPos = pos + drawOrigin;
                // Store off the positions in device space to position the glyphs during drawing.
                sourcePositions.push_back(srcPos);
                SkRect rect = glyph->rect();
                rect.makeOffset(srcPos);
                runMatrix.mapRectToQuad(corners, rect);
                // left top -> right top
                SkScalar scale = (corners[1] - corners[0]).length() / rect.width();
                maxScale = std::max(maxScale, scale);
                // right top -> right bottom
                scale = (corners[2] - corners[1]).length() / rect.height();
                maxScale = std::max(maxScale, scale);
                // right bottom -> left bottom
                scale = (corners[3] - corners[2]).length() / rect.width();
                maxScale = std::max(maxScale, scale);
                // left bottom -> left top
                scale = (corners[0] - corners[3]).length() / rect.height();
                maxScale = std::max(maxScale, scale);
            }

            if (maxScale * runFont.getSize() > 256) {
                maxScale = 256.0f / runFont.getSize();
            }

            SkMatrix cacheScale = SkMatrix::Scale(maxScale, maxScale);
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, cacheScale);

            auto strike = strikeSpec.findOrCreateStrike();

            // Figure out all the positions and packed glyphIDs based on the device matrix.
            fDrawable.startBitmapDevice(
                    fRejects.source(), drawOrigin, deviceMatrix, strike->roundingSpec());

            strike->prepareForDrawingMasksCPU(&fDrawable);
            auto variants = fDrawable.drawable().get<0>();
            for (auto [variant, srcPos] : SkMakeZip(variants, sourcePositions)) {
                SkGlyph* glyph = variant.glyph();
                SkMask mask = glyph->mask();
                // TODO: is this needed will A8 and BW just work?
                if (mask.fFormat != SkMask::kARGB32_Format) {
                    continue;
                }
                SkBitmap bm;
                bm.installPixels(SkImageInfo::MakeN32Premul(mask.fBounds.size()),
                                 mask.fImage,
                                 mask.fRowBytes);

                // Since the glyph in the cache is scaled by maxScale, its top left vector is too
                // long. Reduce it to find proper positions on the device.
                SkPoint realPos = srcPos
                        + SkPoint::Make(mask.fBounds.left(), mask.fBounds.top()) * (1.0f/maxScale);

                // Calculate the preConcat matrix for drawBitmap to get the rectangle from the
                // glyph cache (which is multiplied by maxScale) to land in the right place.
                SkMatrix translate = SkMatrix::Translate(realPos);
                translate.preScale(1.0f/maxScale, 1.0f/maxScale);

                // Draw the bitmap using the rect from the scaled cache, and not the source
                // rectangle for the glyph.
                bitmapDevice->drawBitmap(
                        bm, translate, nullptr, SkSamplingOptions{SkFilterMode::kLinear},
                        paint);
            }
            fRejects.flipRejectsToSource();
        }

        // TODO: have the mask stage above reject the glyphs that are too big, and handle the
        //  rejects in a more sophisticated stage.
    }
}

// Use the following in your args.gn to dump telemetry for diagnosing chrome Renderer/GPU
// differences.
// extra_cflags = ["-D", "SK_TRACE_GLYPH_RUN_PROCESS"]

#if SK_SUPPORT_GPU
void SkGlyphRunListPainter::processGlyphRun(const SkGlyphRun& glyphRun,
                                            const SkMatrix& drawMatrix,
                                            const SkPaint& runPaint,
                                            const GrSDFTControl& control,
                                            SkGlyphRunPainterInterface* process,
                                            const char* tag,
                                            uint64_t uniqueID) {
    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        SkString msg;
        msg.appendf("\nStart glyph run processing");
        if (tag != nullptr) {
            msg.appendf(" for %s ", tag);
            if (uniqueID != SK_InvalidUniqueID) {
                msg.appendf(" uniqueID: %" PRIu64, uniqueID);
            }
        }
        msg.appendf("\n   matrix\n");
        msg.appendf("   %7.3g %7.3g %7.3g\n   %7.3g %7.3g %7.3g\n",
                    drawMatrix[0], drawMatrix[1], drawMatrix[2],
                    drawMatrix[3], drawMatrix[4], drawMatrix[5]);
    #endif
    ScopedBuffers _ = this->ensureBuffers(glyphRun);
    fRejects.setSource(glyphRun.source());
    const SkFont& runFont = glyphRun.font();

    GrSDFTControl::DrawingType drawingType = control.drawingType(runFont, runPaint, drawMatrix);

    if (drawingType == GrSDFTControl::kSDFT) {
        // Process SDFT - This should be the .009% case.
        const auto& [strikeSpec, strikeToSourceScale, minScale, maxScale] =
                SkStrikeSpec::MakeSDFT(runFont, runPaint, fDeviceProps, drawMatrix, control);

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  SDFT case:\n%s", strikeSpec.dump().c_str());
        #endif

        if (!SkScalarNearlyZero(strikeToSourceScale)) {
            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startSource(fRejects.source());
            #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                msg.appendf("    glyphs:(x,y):\n      %s\n", fDrawable.dumpInput().c_str());
            #endif
            strike->prepareForSDFTDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            if (process && !fDrawable.drawableIsEmpty()) {
                // processSourceSDFT must be called even if there are no glyphs to make sure
                // runs are set correctly.
                process->processSourceSDFT(fDrawable.drawable(),
                                           strike->getUnderlyingStrike(),
                                           strikeToSourceScale,
                                           runFont,
                                           minScale, maxScale);
            }
        }
    }

    if (drawingType != GrSDFTControl::kPath && !fRejects.source().empty()) {
        // Process masks including ARGB - this should be the 99.99% case.

        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                runFont, runPaint, fDeviceProps, fScalerContextFlags, drawMatrix);

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Mask case:\n%s", strikeSpec.dump().c_str());
        #endif

        SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

        fDrawable.startGPUDevice(fRejects.source(), drawMatrix, strike->roundingSpec());
        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("    glyphs:(x,y):\n      %s\n", fDrawable.dumpInput().c_str());
        #endif
        strike->prepareForMaskDrawing(&fDrawable, &fRejects);
        fRejects.flipRejectsToSource();

        if (process && !fDrawable.drawableIsEmpty()) {
            // processDeviceMasks must be called even if there are no glyphs to make sure runs
            // are set correctly.
            process->processDeviceMasks(fDrawable.drawable(), strike->getUnderlyingStrike());
        }
    }

    // Glyphs are generated in different scales relative to the source space. Masks are drawn
    // in device space, and SDFT and Paths are draw in a fixed constant space. The
    // maxDimensionInSourceSpace is used to calculate the factor from strike space to source
    // space.
    SkScalar maxDimensionInSourceSpace = 0.0;
    if (!fRejects.source().empty()) {
        // Path case - handle big things without color and that have a path.
        auto [strikeSpec, strikeToSourceScale] =
                SkStrikeSpec::MakePath(runFont, runPaint, fDeviceProps, fScalerContextFlags);

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("  Path case:\n%s", strikeSpec.dump().c_str());
        #endif

        if (!SkScalarNearlyZero(strikeToSourceScale)) {
            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startSource(fRejects.source());
            #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                msg.appendf("    glyphs:(x,y):\n      %s\n", fDrawable.dumpInput().c_str());
            #endif
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();
            maxDimensionInSourceSpace = fRejects.rejectedMaxDimension() * strikeToSourceScale;

            if (process && !fDrawable.drawableIsEmpty()) {
                // processSourcePaths must be called even if there are no glyphs to make sure
                // runs are set correctly.
                process->processSourcePaths(
                        fDrawable.drawable(), runFont, strikeToSourceScale);
            }
        }
    }

    if (!fRejects.source().empty() && maxDimensionInSourceSpace != 0) {
        // Draw of last resort. Scale the bitmap to the screen.
        auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeSourceFallback(
                runFont, runPaint, fDeviceProps,
                fScalerContextFlags, maxDimensionInSourceSpace);

        #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
            msg.appendf("Transformed case:\n%s", strikeSpec.dump().c_str());
        #endif

        if (!SkScalarNearlyZero(strikeToSourceScale)) {
            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startSource(fRejects.source());
            #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
                msg.appendf("glyphs:(x,y):\n      %s\n", fDrawable.dumpInput().c_str());
            #endif
            strike->prepareForMaskDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();
            SkASSERT(fRejects.source().empty());

            if (process && !fDrawable.drawableIsEmpty()) {
                process->processSourceMasks(
                        fDrawable.drawable(), strike->getUnderlyingStrike(), strikeToSourceScale);
            }
        }
    }
    #if defined(SK_TRACE_GLYPH_RUN_PROCESS)
        msg.appendf("End glyph run processing");
        if (tag != nullptr) {
            msg.appendf(" for %s ", tag);
        }
        SkDebugf("%s\n", msg.c_str());
    #endif
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
