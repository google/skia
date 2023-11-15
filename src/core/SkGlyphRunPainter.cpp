/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRunPainter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/text/GlyphRun.h"

#include <algorithm>
#include <initializer_list>
#include <tuple>
#include <vector>

using namespace skia_private;

using namespace skglyph;
using namespace sktext;

namespace {
SkScalerContextFlags compute_scaler_context_flags(const SkColorSpace* cs) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (cs && cs->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

// TODO: collect this up into a single class when all the details are worked out.
// This is duplicate code. The original is in SubRunContainer.cpp.
std::tuple<SkZip<const SkGlyph*, SkPoint>, SkZip<SkGlyphID, SkPoint>>
prepare_for_path_drawing(SkStrike* strike,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<const SkGlyph*, SkPoint> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    strike->lock();
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }
        const SkPackedGlyphID packedID{glyphID};
        switch (SkGlyphDigest digest = strike->digestFor(kPath, packedID);
                digest.actionFor(kPath)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(strike->glyph(digest), pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    strike->unlock();
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

// TODO: collect this up into a single class when all the details are worked out.
// This is duplicate code. The original is in SubRunContainer.cpp.
std::tuple<SkZip<const SkGlyph*, SkPoint>, SkZip<SkGlyphID, SkPoint>>
prepare_for_drawable_drawing(SkStrike* strike,
                             SkZip<const SkGlyphID, const SkPoint> source,
                             SkZip<const SkGlyph*, SkPoint> acceptedBuffer,
                             SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    strike->lock();
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }
        const SkPackedGlyphID packedID{glyphID};
        switch (SkGlyphDigest digest = strike->digestFor(kDrawable, packedID);
                digest.actionFor(kDrawable)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(strike->glyph(digest), pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    strike->unlock();
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

std::tuple<SkZip<const SkGlyph*, SkPoint>, SkZip<SkGlyphID, SkPoint>>
prepare_for_direct_mask_drawing(SkStrike* strike,
                                const SkMatrix& creationMatrix,
                                SkZip<const SkGlyphID, const SkPoint> source,
                                SkZip<const SkGlyph*, SkPoint> acceptedBuffer,
                                SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    const SkIPoint mask = strike->roundingSpec().ignorePositionFieldMask;
    const SkPoint halfSampleFreq = strike->roundingSpec().halfAxisSampleFreq;

    // Build up the mapping from source space to device space. Add the rounding constant
    // halfSampleFreq, so we just need to floor to get the device result.
    SkMatrix positionMatrixWithRounding = creationMatrix;
    positionMatrixWithRounding.postTranslate(halfSampleFreq.x(), halfSampleFreq.y());

    int acceptedSize = 0;
    int rejectedSize = 0;
    strike->lock();
    for (auto [glyphID, pos] : source) {
        if (!SkScalarsAreFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPoint mappedPos = positionMatrixWithRounding.mapPoint(pos);
        const SkPackedGlyphID packedGlyphID = SkPackedGlyphID{glyphID, mappedPos, mask};
        switch (SkGlyphDigest digest = strike->digestFor(kDirectMaskCPU, packedGlyphID);
                digest.actionFor(kDirectMaskCPU)) {
            case GlyphAction::kAccept: {
                const SkPoint roundedPos{SkScalarFloorToScalar(mappedPos.x()),
                                         SkScalarFloorToScalar(mappedPos.y())};
                acceptedBuffer[acceptedSize++] =
                        std::make_tuple(strike->glyph(digest), roundedPos);
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    strike->unlock();

    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}
}  // namespace

// -- SkGlyphRunListPainterCPU ---------------------------------------------------------------------
SkGlyphRunListPainterCPU::SkGlyphRunListPainterCPU(const SkSurfaceProps& props,
                                                   SkColorType colorType,
                                                   SkColorSpace* cs)
        : fDeviceProps{props}
        , fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        , fColorType{colorType}
        , fScalerContextFlags{compute_scaler_context_flags(cs)} {}

void SkGlyphRunListPainterCPU::drawForBitmapDevice(SkCanvas* canvas,
                                                   const BitmapDevicePainter* bitmapDevice,
                                                   const sktext::GlyphRunList& glyphRunList,
                                                   const SkPaint& paint,
                                                   const SkMatrix& drawMatrix) {
    STArray<64, const SkGlyph*> acceptedPackedGlyphIDs;
    STArray<64, SkPoint> acceptedPositions;
    STArray<64, SkGlyphID> rejectedGlyphIDs;
    STArray<64, SkPoint> rejectedPositions;
    const int maxGlyphRunSize = glyphRunList.maxGlyphRunSize();
    acceptedPackedGlyphIDs.resize(maxGlyphRunSize);
    acceptedPositions.resize(maxGlyphRunSize);
    const auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);
    rejectedGlyphIDs.resize(maxGlyphRunSize);
    rejectedPositions.resize(maxGlyphRunSize);
    const auto rejectedBuffer = SkMakeZip(rejectedGlyphIDs, rejectedPositions);

    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicate this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && paint.isSrcOver())
                          ? fDeviceProps
                          : fBitmapFallbackProps;

    SkPoint drawOrigin = glyphRunList.origin();
    SkMatrix positionMatrix{drawMatrix};
    positionMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        SkZip<const SkGlyphID, const SkPoint> source = glyphRun.source();

        if (SkStrikeSpec::ShouldDrawAsPath(paint, runFont, positionMatrix)) {
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, paint, props, fScalerContextFlags);

            auto strike = strikeSpec.findOrCreateStrike();

            {
                auto [accepted, rejected] = prepare_for_path_drawing(strike.get(),
                                                                     source,
                                                                     acceptedBuffer,
                                                                     rejectedBuffer);

                source = rejected;
                // The paint we draw paths with must have the same anti-aliasing state as the
                // runFont allowing the paths to have the same edging as the glyph masks.
                SkPaint pathPaint = paint;
                pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

                const bool stroking = pathPaint.getStyle() != SkPaint::kFill_Style;
                const bool hairline = pathPaint.getStrokeWidth() == 0;
                const bool needsExactCTM = pathPaint.getShader()     ||
                                           pathPaint.getPathEffect() ||
                                           pathPaint.getMaskFilter() ||
                                           (stroking && !hairline);

                if (!needsExactCTM) {
                    for (auto [glyph, pos] : accepted) {
                        const SkPath* path = glyph->path();
                        SkMatrix m;
                        SkPoint translate = drawOrigin + pos;
                        m.setScaleTranslate(strikeToSourceScale, strikeToSourceScale,
                                            translate.x(), translate.y());
                        SkAutoCanvasRestore acr(canvas, true);
                        canvas->concat(m);
                        canvas->drawPath(*path, pathPaint);
                    }
                } else {
                    for (auto [glyph, pos] : accepted) {
                        const SkPath* path = glyph->path();
                        SkMatrix m;
                        SkPoint translate = drawOrigin + pos;
                        m.setScaleTranslate(strikeToSourceScale, strikeToSourceScale,
                                            translate.x(), translate.y());

                        SkPath deviceOutline;
                        path->transform(m, &deviceOutline);
                        deviceOutline.setIsVolatile(true);
                        canvas->drawPath(deviceOutline, pathPaint);
                    }
                }
            }

            if (!source.empty()) {
                auto [accepted, rejected] = prepare_for_drawable_drawing(strike.get(),
                                                                         source,
                                                                         acceptedBuffer,
                                                                         rejectedBuffer);
                source = rejected;

                for (auto [glyph, pos] : accepted) {
                    SkDrawable* drawable = glyph->drawable();
                    SkMatrix m;
                    SkPoint translate = drawOrigin + pos;
                    m.setScaleTranslate(strikeToSourceScale, strikeToSourceScale,
                                        translate.x(), translate.y());
                    SkAutoCanvasRestore acr(canvas, false);
                    SkRect drawableBounds = drawable->getBounds();
                    m.mapRect(&drawableBounds);
                    canvas->saveLayer(&drawableBounds, &paint);
                    drawable->draw(canvas, &m);
                }
            }
        }
        if (!source.empty() && !positionMatrix.hasPerspective()) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, positionMatrix);

            auto strike = strikeSpec.findOrCreateStrike();

            auto [accepted, rejected] = prepare_for_direct_mask_drawing(strike.get(),
                                                                        positionMatrix,
                                                                        source,
                                                                        acceptedBuffer,
                                                                        rejectedBuffer);
            source = rejected;
            bitmapDevice->paintMasks(accepted, paint);
        }
        if (!source.empty()) {
            std::vector<SkPoint> sourcePositions;

            // Create a strike is source space to calculate scale information.
            SkStrikeSpec scaleStrikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, SkMatrix::I());
            SkBulkGlyphMetrics metrics{scaleStrikeSpec};

            auto glyphIDs = source.get<0>();
            auto positions = source.get<1>();
            SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);
            SkScalar maxScale = SK_ScalarMin;

            // Calculate the scale that makes the longest edge 1:1 with its side in the cache.
            for (auto [glyph, pos] : SkMakeZip(glyphs, positions)) {
                if (glyph->isEmpty()) {
                    continue;
                }
                SkPoint corners[4];
                SkPoint srcPos = pos + drawOrigin;
                // Store off the positions in device space to position the glyphs during drawing.
                sourcePositions.push_back(srcPos);
                SkRect rect = glyph->rect();
                rect.makeOffset(srcPos);
                positionMatrix.mapRectToQuad(corners, rect);
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

            if (maxScale <= 0) {
                continue;  // to the next run.
            }

            if (maxScale * runFont.getSize() > 256) {
                maxScale = 256.0f / runFont.getSize();
            }

            SkMatrix cacheScale = SkMatrix::Scale(maxScale, maxScale);
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, paint, props, fScalerContextFlags, cacheScale);

            auto strike = strikeSpec.findOrCreateStrike();

            auto [accepted, rejected] = prepare_for_direct_mask_drawing(strike.get(),
                                                                        positionMatrix,
                                                                        source,
                                                                        acceptedBuffer,
                                                                        rejectedBuffer);
            const SkScalar invMaxScale = 1.0f/maxScale;
            for (auto [glyph, srcPos] : SkMakeZip(accepted.get<0>(), sourcePositions)) {
                SkMask mask = glyph->mask();
                // TODO: is this needed will A8 and BW just work?
                if (mask.fFormat != SkMask::kARGB32_Format) {
                    continue;
                }
                SkBitmap bm;
                bm.installPixels(SkImageInfo::MakeN32Premul(mask.fBounds.size()),
                                 const_cast<uint8_t*>(mask.fImage),
                                 mask.fRowBytes);
                bm.setImmutable();

                // Since the glyph in the cache is scaled by maxScale, its top left vector is too
                // long. Reduce it to find proper positions on the device.
                SkPoint realPos =
                    srcPos + SkPoint::Make(mask.fBounds.left(), mask.fBounds.top())*invMaxScale;

                // Calculate the preConcat matrix for drawBitmap to get the rectangle from the
                // glyph cache (which is multiplied by maxScale) to land in the right place.
                SkMatrix translate = SkMatrix::Translate(realPos);
                translate.preScale(invMaxScale, invMaxScale);

                // Draw the bitmap using the rect from the scaled cache, and not the source
                // rectangle for the glyph.
                bitmapDevice->drawBitmap(bm, translate, nullptr, SkFilterMode::kLinear, paint);
            }
        }

        // TODO: have the mask stage above reject the glyphs that are too big, and handle the
        //  rejects in a more sophisticated stage.
    }
}
