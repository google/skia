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

#include "SkArenaAlloc.h"
#include "SkDescriptor.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSurfaceProps.h"
#include "SkTemplates.h"
#include "SkTextBlobPriv.h"
#include "SkTypes.h"
#if SK_SUPPORT_GPU
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif
class SkBaseDevice;
class SkGlyphRunList;
class SkRasterClip;

template <typename T>
class SkSpan {
public:
    SkSpan() : fPtr{nullptr}, fSize{0} {}
    SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} { }
    template <typename U>
    explicit SkSpan(std::vector<U>& v) : fPtr{v.data()}, fSize{v.size()} {}
    SkSpan(const SkSpan<T>& o) = default;
    SkSpan& operator=( const SkSpan& other ) = default;
    T& operator [] (size_t i) const { return fPtr[i]; }
    T* begin() const { return fPtr; }
    T* end() const { return fPtr + fSize; }
    const T* cbegin() const { return fPtr; }
    const T* cend() const { return fPtr + fSize; }
    T* data() const { return fPtr; }
    size_t size() const { return fSize; }
    bool empty() const { return fSize == 0; }
    size_t size_bytes() const { return fSize * sizeof(T); }
    SkSpan<const T> toConst() const { return SkSpan<const T>{fPtr, fSize}; }

private:
    T* fPtr;
    size_t fSize;
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

class SkGlyphRunListDrawer {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListDrawer(
            const SkSurfaceProps& props, SkColorType colorType, SkScalerContextFlags flags);

    #if SK_SUPPORT_GPU
    SkGlyphRunListDrawer(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListDrawer(const GrRenderTargetContext& renderTargetContext);
    #endif

    using PerGlyph = std::function<void(const SkGlyph&, SkPoint)>;
    using PerMask = std::function<void(const SkMask&, const SkGlyph&, SkPoint)>;
    using PerMaskCreator = std::function<PerMask(const SkPaint&, SkArenaAlloc* alloc)>;
    using PerPath = std::function<void(const SkPath*, const SkGlyph&, SkPoint)>;
    using PerPathCreator = std::function<PerPath(
            const SkPaint&, SkScalar matrixScale,SkArenaAlloc* alloc)>;
    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            PerMaskCreator perMaskCreator, PerPathCreator perPathCreator);
    void drawUsingMasks(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun, SkPoint origin,
            const SkMatrix& deviceMatrix, PerMask perMask);
    void drawUsingPaths(
            const SkGlyphRun& glyphRun, SkPoint origin, SkGlyphCache* cache, PerPath perPath) const;

    void drawGlyphRunAsGlyphWithPathFallback(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerGlyph perGlyph, PerPath perPath);

private:
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix);
    bool ensureBitmapBuffers(size_t runSize);


    template <typename EachGlyph>
    void forEachMappedDrawableGlyph(
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
            SkGlyphCache* cache, EachGlyph eachGlyph);

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

class SkGlyphRunListIterator {
public:
    explicit SkGlyphRunListIterator(const SkGlyphRunList& list) : fList{list} {}

    bool done() const { return fIndex == fList.size(); }
    void next() { fIndex += 1;}
    uint32_t glyphCount() const { return fList[fIndex].runSize(); }
    const uint16_t* glyphs() const { return fList[fIndex].shuntGlyphsIDs().data(); }
    const SkScalar* pos() const { return (const SkScalar*)fList[fIndex].positions().data(); }
    const SkPoint& offset() const { return fZero; }
    void applyFontToPaint(SkPaint* paint) const { *paint = fList[fIndex].paint(); }
    SkTextBlobRunIterator::GlyphPositioning positioning() const {
        return SkTextBlobRunIterator::kFull_Positioning;
    }
    const uint32_t* clusters() const { return fList[fIndex].clusters().data(); }
    uint32_t textSize() const { return fList[fIndex].text().size(); }
    const char* text() const { return fList[fIndex].text().data(); }
    const SkGlyphRun& glyphRun() const { return fList[fIndex]; }

    bool isLCD() const { return fList[fIndex].paint().isLCDRenderText(); }

private:
    static constexpr SkPoint fZero{0, 0};
    size_t fIndex{0};
    const SkGlyphRunList& fList;
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

#endif  // SkGlyphRunInfo_DEFINED
