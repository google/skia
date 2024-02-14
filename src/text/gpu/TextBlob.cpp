/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/TextBlob.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <memory>
#include <utility>

class SkMaskFilter;

using namespace sktext::gpu;
namespace {

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from initial matrix to the position matrix
// support using direct glyph masks.
std::tuple<bool, SkVector> can_use_direct(
        const SkMatrix& initialPositionMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the initialPositionMatrix, and the
    // positionMatrix have the same 2x2, and the translation between them is integer.
    // Calculate the translation in source space to a translation in device space by mapping
    // (0, 0) through both the initial position matrix and the position matrix; take the difference.
    SkVector translation = positionMatrix.mapOrigin() - initialPositionMatrix.mapOrigin();
    return {initialPositionMatrix.getScaleX() == positionMatrix.getScaleX() &&
            initialPositionMatrix.getScaleY() == positionMatrix.getScaleY() &&
            initialPositionMatrix.getSkewX()  == positionMatrix.getSkewX()  &&
            initialPositionMatrix.getSkewY()  == positionMatrix.getSkewY()  &&
            SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()),
            translation};
}


static SkColor compute_canonical_color(const SkPaint& paint, bool lcd) {
    SkColor canonicalColor = SkPaintPriv::ComputeLuminanceColor(paint);
    if (lcd) {
        // This is the correct computation for canonicalColor, but there are tons of cases where LCD
        // can be modified. For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these modifications are and see if we can incorporate that
        //      logic at a higher level *OR* use sRGB
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note above.  We pick a placeholder value for LCD text to ensure we always match
        // the same key
        return SK_ColorTRANSPARENT;
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

}  // namespace

namespace sktext::gpu {
// -- TextBlob::Key ------------------------------------------------------------------------------
auto TextBlob::Key::Make(const GlyphRunList& glyphRunList,
                         const SkPaint& paint,
                         const SkMatrix& drawMatrix,
                         const SkStrikeDeviceInfo& strikeDevice) -> std::tuple<bool, Key> {
    SkASSERT(strikeDevice.fSDFTControl != nullptr);
    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* maskFilter = paint.getMaskFilter();
    bool canCache = glyphRunList.canCache() &&
                    !(paint.getPathEffect() ||
                        (maskFilter && !as_MFB(maskFilter)->asABlur(&blurRec)));

    TextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? strikeDevice.fSurfaceProps.pixelGeometry()
                                               : kUnknown_SkPixelGeometry;

        SkColor canonicalColor = compute_canonical_color(paint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = paint.getStyle();
        if (key.fStyle != SkPaint::kFill_Style) {
            key.fFrameWidth = paint.getStrokeWidth();
            key.fMiterLimit = paint.getStrokeMiter();
            key.fJoin = paint.getStrokeJoin();
        }
        key.fHasBlur = maskFilter != nullptr;
        if (key.fHasBlur) {
            key.fBlurRec = blurRec;
        }
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = SkTo<uint32_t>(strikeDevice.fScalerContextFlags);

        // Do any runs use direct drawing types?.
        key.fHasSomeDirectSubRuns = false;
        SkPoint glyphRunListLocation = glyphRunList.sourceBoundsWithOrigin().center();
        for (auto& run : glyphRunList) {
            SkScalar approximateDeviceTextSize =
                    SkFontPriv::ApproximateTransformedTextSize(run.font(), drawMatrix,
                                                               glyphRunListLocation);
            key.fHasSomeDirectSubRuns |=
                    strikeDevice.fSDFTControl->isDirect(approximateDeviceTextSize, paint,
                                                        drawMatrix);
        }

        if (key.fHasSomeDirectSubRuns) {
            // Store the fractional offset of the position. We know that the matrix can't be
            // perspective at this point.
            SkPoint mappedOrigin = drawMatrix.mapOrigin();
            key.fPositionMatrix = drawMatrix;
            key.fPositionMatrix.setTranslateX(
                    mappedOrigin.x() - SkScalarFloorToScalar(mappedOrigin.x()));
            key.fPositionMatrix.setTranslateY(
                    mappedOrigin.y() - SkScalarFloorToScalar(mappedOrigin.y()));
        } else {
            // For path and SDFT, the matrix doesn't matter.
            key.fPositionMatrix = SkMatrix::I();
        }
    }

    return {canCache, key};
}

bool TextBlob::Key::operator==(const TextBlob::Key& that) const {
    if (fUniqueID != that.fUniqueID) { return false; }
    if (fCanonicalColor != that.fCanonicalColor) { return false; }
    if (fStyle != that.fStyle) { return false; }
    if (fStyle != SkPaint::kFill_Style) {
        if (fFrameWidth != that.fFrameWidth ||
            fMiterLimit != that.fMiterLimit ||
            fJoin != that.fJoin) {
            return false;
        }
    }
    if (fPixelGeometry != that.fPixelGeometry) { return false; }
    if (fHasBlur != that.fHasBlur) { return false; }
    if (fHasBlur) {
        if (fBlurRec.fStyle != that.fBlurRec.fStyle || fBlurRec.fSigma != that.fBlurRec.fSigma) {
            return false;
        }
    }

    if (fScalerContextFlags != that.fScalerContextFlags) { return false; }

    // DirectSubRuns do not support perspective when used with a TextBlob. SDFT, Transformed,
    // Path, and Drawable do support perspective.
    if (fPositionMatrix.hasPerspective() && fHasSomeDirectSubRuns) { return false; }

    if (fHasSomeDirectSubRuns != that.fHasSomeDirectSubRuns) { return false; }

    if (fHasSomeDirectSubRuns) {
        auto [compatible, _] = can_use_direct(fPositionMatrix, that.fPositionMatrix);
        return compatible;
    }

    return true;
}

// -- TextBlob -----------------------------------------------------------------------------------
void TextBlob::operator delete(void* p) { ::operator delete(p); }
void* TextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* TextBlob::operator new(size_t, void* p) { return p; }

TextBlob::~TextBlob() = default;

sk_sp<TextBlob> TextBlob::Make(const GlyphRunList& glyphRunList,
                               const SkPaint& paint,
                               const SkMatrix& positionMatrix,
                               SkStrikeDeviceInfo strikeDeviceInfo,
                               StrikeForGPUCacheInterface* strikeCache) {
    size_t subRunSizeHint = SubRunContainer::EstimateAllocSize(glyphRunList);
    auto [initializer, totalMemoryAllocated, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<TextBlob>(subRunSizeHint);

    auto container = SubRunContainer::MakeInAlloc(
            glyphRunList, positionMatrix, paint,
            strikeDeviceInfo, strikeCache, &alloc, SubRunContainer::kAddSubRuns, "TextBlob");

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(paint);
    sk_sp<TextBlob> blob = sk_sp<TextBlob>(initializer.initialize(std::move(alloc),
                                                                  std::move(container),
                                                                  totalMemoryAllocated,
                                                                  initialLuminance));
    return blob;
}

void TextBlob::addKey(const Key& key) {
    fKey = key;
}

bool TextBlob::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    // A singular matrix will create a TextBlob with no SubRuns, but unknown glyphs can also
    // cause empty runs. If there are no subRuns, then regenerate when the matrices don't match.
    if (fSubRuns->isEmpty() && fSubRuns->initialPosition() != positionMatrix) {
        return false;
    }

    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fInitialLuminance != SkPaintPriv::ComputeLuminanceColor(paint)) {
        return false;
    }

    return fSubRuns->canReuse(paint, positionMatrix);
}

const TextBlob::Key& TextBlob::key() const { return fKey; }

void TextBlob::draw(SkCanvas* canvas,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    const AtlasDrawDelegate& atlasDelegate) {
    fSubRuns->draw(canvas, drawOrigin, paint, this, atlasDelegate);
}

TextBlob::TextBlob(SubRunAllocator&& alloc,
                   SubRunContainerOwner subRuns,
                   int totalMemorySize,
                   SkColor initialLuminance)
        : fAlloc{std::move(alloc)}
        , fSubRuns{std::move(subRuns)}
        , fSize(totalMemorySize)
        , fInitialLuminance{initialLuminance} { }

sk_sp<Slug> MakeSlug(const SkMatrix& drawMatrix,
                     const sktext::GlyphRunList& glyphRunList,
                     const SkPaint& initialPaint,
                     const SkPaint& drawingPaint,
                     SkStrikeDeviceInfo strikeDeviceInfo,
                     sktext::StrikeForGPUCacheInterface* strikeCache) {
    return SlugImpl::Make(
            drawMatrix, glyphRunList, initialPaint, drawingPaint, strikeDeviceInfo, strikeCache);
}
}  // namespace sktext::gpu
