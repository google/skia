/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunInfo_DEFINED
#define SkGlyphRunInfo_DEFINED

#include <functional>
#include <memory>
#include <vector>

#include "SkDescriptor.h"
#include "SkDistanceFieldGen.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSpan.h"
#include "SkSurfaceProps.h"
#include "SkTemplates.h"
#include "SkTextBlobPriv.h"
#include "SkTypes.h"

class SkArenaAlloc;
class SkBaseDevice;
class SkGlyphRunList;
class SkRasterClip;

#if SK_SUPPORT_GPU
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkGlyphCacheInterface {
public:
    virtual ~SkGlyphCacheInterface() = default;
    virtual SkVector rounding() const = 0;
    virtual const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) = 0;
};

class SkGlyphCacheCommon {
public:
    static SkVector PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment) {
        if (!isSubpixel) {
            return {SK_ScalarHalf, SK_ScalarHalf};
        } else {
            static constexpr SkScalar kSubpixelRounding = SkFixedToScalar(SkGlyph::kSubpixelRound);
            switch (axisAlignment) {
                case kX_SkAxisAlignment:
                    return {kSubpixelRounding, SK_ScalarHalf};
                case kY_SkAxisAlignment:
                    return {SK_ScalarHalf, kSubpixelRounding};
                case kNone_SkAxisAlignment:
                    return {kSubpixelRounding, kSubpixelRounding};
            }
        }

        // Some compilers need this.
        return {0, 0};
    }

    // This assumes that position has the appropriate rounding term applied.
    static SkIPoint SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position) {
        // TODO: SkScalarFraction uses truncf to calculate the fraction. This should be floorf.
        SkFixed lookupX = SkScalarToFixed(SkScalarFraction(position.x())),
                lookupY = SkScalarToFixed(SkScalarFraction(position.y()));

        // Snap to a given axis if alignment is requested.
        if (axisAlignment == kX_SkAxisAlignment) {
            lookupY = 0;
        } else if (axisAlignment == kY_SkAxisAlignment) {
            lookupX = 0;
        }

        return {lookupX, lookupY};
    }
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkPaint&& runPaint,
               SkSpan<const uint16_t> denseIndices,
               SkSpan<const SkPoint> positions,
               SkSpan<const SkGlyphID> glyphIDs,
               SkSpan<const SkGlyphID> uniqueGlyphIDs,
               SkSpan<const char> text,
               SkSpan<const uint32_t> clusters);

    // A function that turns an SkGlyphRun into an SkGlyphRun for each glyph.
    using PerGlyph = std::function<void (SkGlyphRun*, SkPaint*)>;
    void eachGlyphToGlyphRun(PerGlyph perGlyph);

    // The following made a ~5% speed improvement over not using a template.
    //using PerGlyphPos = std::function<void (SkGlyphID glyphID, SkPoint positions)>;
    template <typename PerGlyphPos>
    void forEachGlyphAndPosition(PerGlyphPos perGlyph) const;

    // The temporaryShunt calls are to allow inter-operating with existing code while glyph runs
    // are developed.
    void temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin);
    using TemporaryShuntCallback = std::function<void(size_t, const char*, const SkScalar*)>;
    void temporaryShuntToCallback(TemporaryShuntCallback callback);
    void filloutGlyphsAndPositions(SkGlyphID* glyphIDs, SkPoint* positions);

    size_t runSize() const { return fGlyphIDs.size(); }
    SkSpan<const SkPoint> positions() const { return fPositions.toConst(); }
    SkSpan<const SkGlyphID> shuntGlyphsIDs() const { return fGlyphIDs; }
    const SkPaint& paint() const { return fRunPaint; }
    SkPaint* mutablePaint() { return &fRunPaint; }
    SkSpan<const uint32_t> clusters() const { return fClusters; }
    SkSpan<const char> text() const { return fText; }

private:
    //
    const SkSpan<const uint16_t> fUniqueGlyphIDIndices;
    //
    const SkSpan<const SkPoint> fPositions;
    // This is temporary while converting from the old per glyph code to the bulk code.
    const SkSpan<const SkGlyphID> fGlyphIDs;
    // The unique glyphs from fGlyphIDs.
    const SkSpan<const SkGlyphID> fUniqueGlyphIDs;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Paint for this run modified to have glyph encoding and left alignment.
    SkPaint fRunPaint;
};

class SkGlyphRunListPainter {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListPainter(
            const SkSurfaceProps& props, SkColorType colorType, SkScalerContextFlags flags);

    #if SK_SUPPORT_GPU
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
    #endif

    using PerMask = std::function<void(const SkMask&, const SkGlyph&, SkPoint)>;
    using PerMaskCreator = std::function<PerMask(const SkPaint&, SkArenaAlloc* alloc)>;
    using PerPath = std::function<void(const SkPath*, const SkGlyph&, SkPoint)>;
    using PerPathCreator = std::function<PerPath(
            const SkPaint&, SkScalar matrixScale, SkArenaAlloc* alloc)>;
    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            PerMaskCreator perMaskCreator, PerPathCreator perPathCreator);
    void drawUsingMasks(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun, SkPoint origin,
            const SkMatrix& deviceMatrix, PerMask perMask);
    void drawUsingPaths(
            const SkGlyphRun& glyphRun, SkPoint origin, SkGlyphCache* cache, PerPath perPath) const;

    //using PerGlyph = std::function<void(const SkGlyph&, SkPoint)>;
    template <typename PerGlyphT, typename PerPathT>
    void drawGlyphRunAsBMPWithPathFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerGlyphT perGlyph, PerPathT perPath);

    template <typename PerSDFT, typename PerPathT, typename PerFallbackT>
    void drawGlyphRunAsSDFWithFallback(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, SkScalar textRatio,
            PerSDFT perSDF, PerPathT perPath, PerFallbackT perFallback);

private:
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix);
    bool ensureBitmapBuffers(size_t runSize);


    template <typename EachGlyph>
    void forEachMappedDrawableGlyph(
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
            SkGlyphCacheInterface* cache, EachGlyph eachGlyph);

    void drawGlyphRunAsSubpixelMask(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerMask perMask);

    void drawGlyphRunAsFullpixelMask(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerMask perMask);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;
    size_t fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
};

class SkGlyphRunList {
    const SkPaint* fOriginalPaint{nullptr};  // This should be deleted soon.
    // The text blob is needed to hookup the call back that the SkTextBlob destructor calls. It
    // should be used for nothing else
    const SkTextBlob*  fOriginalTextBlob{nullptr};
    SkPoint fOrigin = {0, 0};
    SkSpan<SkGlyphRun> fGlyphRuns;

public:
    SkGlyphRunList();
    // Blob maybe null.
    SkGlyphRunList(
            const SkPaint& paint,
            const SkTextBlob* blob,
            SkPoint origin,
            SkSpan<SkGlyphRun> glyphRunList);

    SkGlyphRunList(SkGlyphRun* glyphRun);

    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    void temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const;

    bool canCache() const { return fOriginalTextBlob != nullptr; }
    size_t runCount() const { return fGlyphRuns.size(); }
    size_t totalGlyphCount() const {
        size_t glyphCount = 0;
        for(const auto& run : fGlyphRuns) {
            glyphCount += run.runSize();
        }
        return glyphCount;
    }

    SkPoint origin() const { return fOrigin; }
    const SkPaint& paint() const { return *fOriginalPaint; }
    const SkTextBlob* blob() const { return fOriginalTextBlob; }

    auto begin() -> decltype(fGlyphRuns.begin())               { return fGlyphRuns.begin();  }
    auto end()   -> decltype(fGlyphRuns.end())                 { return fGlyphRuns.end();    }
    auto begin() const -> decltype(fGlyphRuns.cbegin())        { return fGlyphRuns.cbegin(); }
    auto end()   const -> decltype(fGlyphRuns.cend())          { return fGlyphRuns.cend();   }
    auto size()  const -> decltype(fGlyphRuns.size())          { return fGlyphRuns.size();   }
    auto empty() const -> decltype(fGlyphRuns.empty())         { return fGlyphRuns.empty();  }
    auto operator [] (size_t i) const -> decltype(fGlyphRuns[i]) { return fGlyphRuns[i];     }
    void temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin) const {
        for (auto& run : fGlyphRuns) {
            run.temporaryShuntToDrawPosText(device, origin);
        }
    }
};

class SkGlyphIDSet {
public:
    SkSpan<const SkGlyphID> uniquifyGlyphIDs(
            uint32_t universeSize, SkSpan<const SkGlyphID> glyphIDs,
            SkGlyphID* uniqueGlyphIDs, uint16_t* denseindices);
private:
    size_t fUniverseToUniqueSize{0};
    SkAutoTMalloc<uint16_t> fUniverseToUnique;
};

class SkGlyphRunBuilder {
public:
    void drawTextAtOrigin(const SkPaint& paint, const void* bytes, size_t byteLength);
    void drawText(
            const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin);
    void drawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar* xpos, SkScalar constY);
    void drawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength, const SkPoint* pos);
    void drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin);
    void drawGlyphPos(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos);

    const SkGlyphRunList& useGlyphRunList();

private:
    void initialize(size_t totalRunSize);
    SkSpan<const SkGlyphID> textToGlyphIDs(
            const SkPaint& paint, const void* bytes, size_t byteLength);

    // Returns the span of unique glyph IDs.
    SkSpan<const SkGlyphID> addDenseAndUnique(
            const SkPaint& paint,
            SkSpan<const SkGlyphID> glyphIDs,
            uint16_t* uniqueGlyphIDIndices,
            SkGlyphID* uniqueGlyphIDs);

    void makeGlyphRun(
            const SkPaint& runPaint,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const uint16_t> uniqueGlyphIDIndices,
            SkSpan<const SkGlyphID> uniqueGlyphIDs,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters);

    void makeGlyphRunList(const SkPaint& paint, const SkTextBlob* blob, SkPoint origin);

    size_t simplifyDrawText(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    size_t simplifyDrawPosTextH(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs,
            const SkScalar* xpos, SkScalar constY,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    size_t simplifyDrawPosText(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});

    size_t fMaxTotalRunSize{0};
    SkAutoTMalloc<uint16_t> fUniqueGlyphIDIndices;
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<SkGlyphID> fUniqueGlyphIDs;

    std::vector<SkGlyphRun> fGlyphRunListStorage;
    SkGlyphRunList fGlyphRunList;


    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint> fScratchAdvances;

    // Used for collecting the set of unique glyphs.
    SkGlyphIDSet fGlyphIDSet;
};

template <typename PerGlyphPos>
inline void SkGlyphRun::forEachGlyphAndPosition(PerGlyphPos perGlyph) const {
    const SkPoint* ptCursor = fPositions.data();
    for (auto glyphID : fGlyphIDs) {
        perGlyph(glyphID, *ptCursor++);
    }
}

inline static bool glyph_too_big_for_atlas(const SkGlyph& glyph) {
    return glyph.fWidth > 256 || glyph.fHeight > 256;
}

inline static SkRect rect_to_draw(
        const SkGlyph& glyph, SkPoint origin, SkScalar textScale, bool isDFT) {

    SkScalar dx = SkIntToScalar(glyph.fLeft);
    SkScalar dy = SkIntToScalar(glyph.fTop);
    SkScalar width = SkIntToScalar(glyph.fWidth);
    SkScalar height = SkIntToScalar(glyph.fHeight);

    if (isDFT) {
        dx += SK_DistanceFieldInset;
        dy += SK_DistanceFieldInset;
        width -= 2 * SK_DistanceFieldInset;
        height -= 2 * SK_DistanceFieldInset;
    }

    dx *= textScale;
    dy *= textScale;
    width *= textScale;
    height *= textScale;

    return SkRect::MakeXYWH(origin.x() + dx, origin.y() + dy, width, height);
}

// forEachMappedDrawableGlyph handles positioning for mask type glyph handling for both sub-pixel
// and full pixel positioning.
template <typename EachGlyph>
void SkGlyphRunListPainter::forEachMappedDrawableGlyph(
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
        SkGlyphCacheInterface* cache, EachGlyph eachGlyph) {
    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    SkVector rounding = cache->rounding();
    mapping.postTranslate(rounding.x(), rounding.y());

    auto runSize = glyphRun.runSize();
    if (this->ensureBitmapBuffers(runSize)) {
        mapping.mapPoints(fPositions, glyphRun.positions().data(), runSize);
        const SkPoint* mappedPtCursor = fPositions;
        const SkPoint* ptCursor = glyphRun.positions().data();
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto mappedPt = *mappedPtCursor++;
            auto pt = origin + *ptCursor++;
            if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
                const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, mappedPt);
                eachGlyph(glyph, pt, mappedPt);
            }
        }
    }
}

template <typename PerGlyphT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerGlyphT perGlyph, PerPathT perPath) {

    auto eachGlyph =
            [perGlyph{std::move(perGlyph)}, perPath{std::move(perPath)}]
                    (const SkGlyph& glyph, SkPoint pt, SkPoint mappedPt) {
                if (glyph_too_big_for_atlas(glyph)) {
                    SkScalar sx = SkScalarFloorToScalar(mappedPt.fX),
                            sy = SkScalarFloorToScalar(mappedPt.fY);

                    SkRect glyphRect =
                            rect_to_draw(glyph, {sx, sy}, SK_Scalar1, false);

                    if (!glyphRect.isEmpty()) {
                        perPath(glyph, mappedPt);
                    }
                } else {
                    perGlyph(glyph, mappedPt);
                }
            };

    this->forEachMappedDrawableGlyph(glyphRun, origin, deviceMatrix, cache, eachGlyph);
}

#endif  // SkGlyphRunInfo_DEFINED
