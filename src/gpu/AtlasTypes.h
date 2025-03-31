/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_AtlasTypes_DEFINED
#define skgpu_AtlasTypes_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerSkyline.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>

class GrOpFlushState;
class SkAutoPixmapStorage;
class TestingUploadTarget;
namespace skgpu::graphite { class RecorderPriv; }

/**
 * This file includes internal types that are used by all of our gpu backends for atlases.
 */

namespace skgpu {

struct IRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    [[nodiscard]] static IRect16 MakeEmpty() {
        IRect16 r;
        r.setEmpty();
        return r;
    }

    [[nodiscard]] static IRect16 MakeWH(int16_t w, int16_t h) {
        IRect16 r;
        r.set(0, 0, w, h);
        return r;
    }

    [[nodiscard]] static IRect16 MakeXYWH(int16_t x, int16_t y, int16_t w, int16_t h) {
        IRect16 r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    [[nodiscard]] static IRect16 Make(const SkIRect& ir) {
        IRect16 r;
        r.set(ir);
        return r;
    }

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void set(const SkIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }

    void offset(int16_t dx, int16_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }
};

/**
 *  Formats for masks, used by the font cache. Important that these are 0-based.
 */
enum class MaskFormat : int {
    kA8,    //!< 1-byte per pixel
    kA565,  //!< 2-bytes per pixel, RGB represent 3-channel LCD coverage
    kARGB,  //!< 4-bytes per pixel, color format

    kLast = kARGB
};
static const int kMaskFormatCount = static_cast<int>(MaskFormat::kLast) + 1;

/**
 *  Return the number of bytes-per-pixel for the specified mask format.
 */
inline constexpr int MaskFormatBytesPerPixel(MaskFormat format) {
    SkASSERT(static_cast<int>(format) < kMaskFormatCount);
    // kA8   (0) -> 1
    // kA565 (1) -> 2
    // kARGB (2) -> 4
    static_assert(static_cast<int>(MaskFormat::kA8) == 0, "enum_order_dependency");
    static_assert(static_cast<int>(MaskFormat::kA565) == 1, "enum_order_dependency");
    static_assert(static_cast<int>(MaskFormat::kARGB) == 2, "enum_order_dependency");

    return SkTo<int>(1u << static_cast<int>(format));
}

static constexpr SkColorType MaskFormatToColorType(MaskFormat format) {
    switch (format) {
        case MaskFormat::kA8:
            return kAlpha_8_SkColorType;
        case MaskFormat::kA565:
            return kRGB_565_SkColorType;
        case MaskFormat::kARGB:
            return kRGBA_8888_SkColorType;
    }
    SkUNREACHABLE;
}

/**
 * Keep track of generation number for atlases and Plots.
 */
class AtlasGenerationCounter {
public:
    inline static constexpr uint64_t kInvalidGeneration = 0;
    uint64_t next() {
        return fGeneration++;
    }

private:
    uint64_t fGeneration{1};
};

/**
 * AtlasToken is used to sequence uploads relative to each other and to batches of draws.
 */
class AtlasToken {
public:
    static AtlasToken InvalidToken() { return AtlasToken(0); }

    AtlasToken(const AtlasToken&) = default;
    AtlasToken& operator=(const AtlasToken&) = default;

    bool operator==(const AtlasToken& that) const {
        return fSequenceNumber == that.fSequenceNumber;
    }
    bool operator!=(const AtlasToken& that) const { return !(*this == that); }
    bool operator<(const AtlasToken that) const {
        return fSequenceNumber < that.fSequenceNumber;
    }
    bool operator<=(const AtlasToken that) const {
        return fSequenceNumber <= that.fSequenceNumber;
    }
    bool operator>(const AtlasToken that) const {
        return fSequenceNumber > that.fSequenceNumber;
    }
    bool operator>=(const AtlasToken that) const {
        return fSequenceNumber >= that.fSequenceNumber;
    }

    AtlasToken& operator++() {
        ++fSequenceNumber;
        return *this;
    }
    AtlasToken operator++(int) {
        auto old = fSequenceNumber;
        ++fSequenceNumber;
        return AtlasToken(old);
    }

    AtlasToken next() const { return AtlasToken(fSequenceNumber + 1); }

    /** Is this token in the [start, end] inclusive interval? */
    bool inInterval(const AtlasToken& start, const AtlasToken& end) {
        return *this >= start && *this <= end;
    }

private:
    AtlasToken() = delete;
    explicit AtlasToken(uint64_t sequenceNumber) : fSequenceNumber(sequenceNumber) {}
    uint64_t fSequenceNumber;
};

/**
 * The TokenTracker encapsulates the incrementing and distribution of AtlasTokens.
 */
class TokenTracker {
public:
    /**
     * Gets the token one beyond the last token that has been flushed,
     * either in GrDrawingManager::flush() or Device::flushPendingWorkToRecorder()
     */
    AtlasToken nextFlushToken() const { return fCurrentFlushToken.next(); }

    /**
     * Gets the next draw token. This can be used to record that the next draw
     * issued will use a resource (e.g. texture) while preparing that draw.
     * Not used by Graphite.
     */
    AtlasToken nextDrawToken() const { return fCurrentDrawToken.next(); }

private:
    // Only these classes get to increment the token counters
    friend class ::GrOpFlushState;
    friend class ::TestingUploadTarget;
    friend class skgpu::graphite::RecorderPriv;

    // Issues the next token for a draw.
    AtlasToken issueDrawToken() { return ++fCurrentDrawToken; }

    // Advances the next token for a flush.
    AtlasToken issueFlushToken() { return ++fCurrentFlushToken; }

    AtlasToken fCurrentDrawToken = AtlasToken::InvalidToken();
    AtlasToken fCurrentFlushToken = AtlasToken::InvalidToken();
};

/**
 * A PlotLocator specifies the plot and is analogous to a directory path:
 *    page/plot/plotGeneration
 *
 * In fact PlotLocator is a portion of a glyph image location in the atlas fully specified by:
 *    format/atlasGeneration/page/plot/plotGeneration/rect
 */
class PlotLocator {
public:
    // These are both restricted by the space they occupy in the PlotLocator.
    // maxPages is also limited by being crammed into the glyph uvs.
    // maxPlots is also limited by the fPlotAlreadyUpdated bitfield in
    // GrDrawOpAtlas::BulkUseTokenUpdater.
    inline static constexpr auto kMaxMultitexturePages = 4;
    inline static constexpr int kMaxPlots = 32;

    PlotLocator(uint32_t pageIdx, uint32_t plotIdx, uint64_t generation)
            : fGenID(generation)
            , fPlotIndex(plotIdx)
            , fPageIndex(pageIdx) {
        SkASSERT(pageIdx < kMaxMultitexturePages);
        SkASSERT(plotIdx < kMaxPlots);
        SkASSERT(generation < ((uint64_t)1 << 48));
    }

    PlotLocator()
            : fGenID(AtlasGenerationCounter::kInvalidGeneration)
            , fPlotIndex(0)
            , fPageIndex(0) {}

    bool isValid() const {
        return fGenID != AtlasGenerationCounter::kInvalidGeneration ||
               fPlotIndex != 0 || fPageIndex != 0;
    }

    void makeInvalid() {
        fGenID = AtlasGenerationCounter::kInvalidGeneration;
        fPlotIndex = 0;
        fPageIndex = 0;
    }

    bool operator==(const PlotLocator& other) const {
        return fGenID == other.fGenID &&
               fPlotIndex == other.fPlotIndex &&
               fPageIndex == other.fPageIndex; }

    uint32_t pageIndex() const { return fPageIndex; }
    uint32_t plotIndex() const { return fPlotIndex; }
    uint64_t genID() const { return fGenID; }

private:
    uint64_t fGenID:48;
    uint64_t fPlotIndex:8;
    uint64_t fPageIndex:8;
};

// AtlasLocator handles atlas position information. It keeps a left-top, right-bottom pair of
// encoded UV coordinates. The bits 13 & 14 of the U coordinates hold the atlas page index.
// This information is handed directly as is from fUVs. This encoding has the nice property
// that width = fUVs[2] - fUVs[0]; the page encoding in the top bits subtracts to zero.
class AtlasLocator {
public:
    std::array<uint16_t, 4> getUVs() const {
        return fUVs;
    }

    void invalidatePlotLocator() { fPlotLocator.makeInvalid(); }

    // TODO: Remove the small path renderer's use of this for eviction
    PlotLocator plotLocator() const { return fPlotLocator; }

    uint32_t pageIndex() const { return fPlotLocator.pageIndex(); }

    uint32_t plotIndex() const { return fPlotLocator.plotIndex(); }

    uint64_t genID() const { return fPlotLocator.genID(); }

    SkIPoint topLeft() const {
        return {fUVs[0] & 0x1FFF, fUVs[1]};
    }

    SkPoint widthHeight() const {
        auto width =  fUVs[2] - fUVs[0],
             height = fUVs[3] - fUVs[1];
        return SkPoint::Make(width, height);
    }

    uint16_t width() const {
        return fUVs[2] - fUVs[0];
    }

    uint16_t height() const {
        return fUVs[3] - fUVs[1];
    }

    void insetSrc(int padding) {
        SkASSERT(2 * padding <= this->width());
        SkASSERT(2 * padding <= this->height());

        fUVs[0] += padding;
        fUVs[1] += padding;
        fUVs[2] -= padding;
        fUVs[3] -= padding;
    }

    void updatePlotLocator(PlotLocator p) {
        fPlotLocator = p;
        SkASSERT(fPlotLocator.pageIndex() <= 3);
        uint16_t page = fPlotLocator.pageIndex() << 13;
        fUVs[0] = (fUVs[0] & 0x1FFF) | page;
        fUVs[2] = (fUVs[2] & 0x1FFF) | page;
    }

    void updateRect(skgpu::IRect16 rect) {
        SkASSERT(rect.fLeft <= rect.fRight);
        SkASSERT(rect.fRight <= 0x1FFF);
        fUVs[0] = (fUVs[0] & 0xE000) | rect.fLeft;
        fUVs[1] = rect.fTop;
        fUVs[2] = (fUVs[2] & 0xE000) | rect.fRight;
        fUVs[3] = rect.fBottom;
    }

private:
    PlotLocator fPlotLocator{AtlasGenerationCounter::kInvalidGeneration, 0, 0};

    // The inset padded bounds in the atlas in the lower 13 bits, and page index in bits 13 &
    // 14 of the Us.
    std::array<uint16_t, 4> fUVs{0, 0, 0, 0};
};

/**
 * An interface for eviction callbacks. Whenever an atlas evicts a specific PlotLocator,
 * it will call all of the registered listeners so they can process the eviction.
 */
class PlotEvictionCallback {
public:
    virtual ~PlotEvictionCallback() = default;
    virtual void evict(PlotLocator) = 0;
};

/**
 * A class which can be handed back to an atlas for updating plots in bulk.  The
 * current max number of plots per page an atlas can handle is 32. If in the future
 * this is insufficient then we can move to a 64 bit int.
 */
class BulkUsePlotUpdater {
public:
    BulkUsePlotUpdater() {
        memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
    }
    BulkUsePlotUpdater(const BulkUsePlotUpdater& that)
            : fPlotsToUpdate(that.fPlotsToUpdate) {
        memcpy(fPlotAlreadyUpdated, that.fPlotAlreadyUpdated, sizeof(fPlotAlreadyUpdated));
    }

    bool add(const skgpu::AtlasLocator& atlasLocator) {
        int plotIdx = atlasLocator.plotIndex();
        int pageIdx = atlasLocator.pageIndex();
        if (this->find(pageIdx, plotIdx)) {
            return false;
        }
        this->set(pageIdx, plotIdx);
        return true;
    }

    void reset() {
        fPlotsToUpdate.clear();
        memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
    }

    struct PlotData {
        PlotData(int pageIdx, int plotIdx) : fPageIndex(pageIdx), fPlotIndex(plotIdx) {}
        uint32_t fPageIndex;
        uint32_t fPlotIndex;
    };

    int count() const { return fPlotsToUpdate.size(); }

    const PlotData& plotData(int index) const { return fPlotsToUpdate[index]; }

private:
    bool find(int pageIdx, int index) const {
        SkASSERT(index < skgpu::PlotLocator::kMaxPlots);
        return (fPlotAlreadyUpdated[pageIdx] >> index) & 1;
    }

    void set(int pageIdx, int index) {
        SkASSERT(!this->find(pageIdx, index));
        fPlotAlreadyUpdated[pageIdx] |= (1 << index);
        fPlotsToUpdate.push_back(PlotData(pageIdx, index));
    }

    inline static constexpr int kMinItems = 4;
    skia_private::STArray<kMinItems, PlotData, true> fPlotsToUpdate;
    // TODO: increase this to uint64_t to allow more plots per page
    uint32_t fPlotAlreadyUpdated[skgpu::PlotLocator::kMaxMultitexturePages];
};

/**
 * The backing texture for an atlas is broken into a spatial grid of Plots. The Plots
 * keep track of subimage placement via their Rectanizer. A Plot may be subclassed if
 * the atlas class needs to track additional information.
 */
class Plot : public SkRefCnt {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);

public:
    Plot(int pageIndex, int plotIndex, AtlasGenerationCounter* generationCounter,
         int offX, int offY, int width, int height, SkColorType colorType, size_t bpp);

    uint32_t pageIndex() const { return fPageIndex; }

    /** plotIndex() is a unique id for the plot relative to the owning GrAtlas and page. */
    uint32_t plotIndex() const { return fPlotIndex; }
    /**
     * genID() is incremented when the plot is evicted due to a atlas spill. It is used to
     * know if a particular subimage is still present in the atlas.
     */
    uint64_t genID() const { return fGenID; }
    PlotLocator plotLocator() const {
        SkASSERT(fPlotLocator.isValid());
        return fPlotLocator;
    }
    SkDEBUGCODE(size_t bpp() const { return fBytesPerPixel; })

    /**
     * To add data to the Plot, first call addRect to see if it's possible. If successful,
     * use the atlasLocator to get a pointer to the location in the atlas via dataAt() and render to
     * that location, or if you already have data use copySubImage().
     */
    bool addRect(int width, int height, AtlasLocator* atlasLocator);
    void* dataAt(const AtlasLocator& atlasLocator);
    void copySubImage(const AtlasLocator& atlasLocator, const void* image);
    // Reset Pixmap to point to backing data for this Plot,
    // and return render location specified by AtlasLocator but relative to this Plot.
    SkIPoint prepForRender(const AtlasLocator&, SkAutoPixmapStorage*);
    // TODO: Utility method for Ganesh, consider removing
    bool addSubImage(int width, int height, const void* image, AtlasLocator* atlasLocator);

    /**
     * To manage the lifetime of a plot, we use two tokens. We use the last upload token to
     * know when we can 'piggy back' uploads, i.e. if the last upload hasn't been flushed to
     * the gpu, we don't need to issue a new upload even if we update the cpu backing store. We
     * use lastUse to determine when we can evict a plot from the cache, i.e. if the last use
     * has already flushed through the gpu then we can reuse the plot.
     */
    skgpu::AtlasToken lastUploadToken() const { return fLastUpload; }
    skgpu::AtlasToken lastUseToken() const { return fLastUse; }
    void setLastUploadToken(skgpu::AtlasToken token) { fLastUpload = token; }
    void setLastUseToken(skgpu::AtlasToken token) { fLastUse = token; }

    int flushesSinceLastUsed() { return fFlushesSinceLastUse; }
    void resetFlushesSinceLastUsed() { fFlushesSinceLastUse = 0; }
    void incFlushesSinceLastUsed() { fFlushesSinceLastUse++; }

    bool needsUpload() { return !fDirtyRect.isEmpty(); }
    std::pair<const void*, SkIRect> prepareForUpload();
    // Re-initialize Plot. The client should ensure that they process any eviction callbacks
    // before calling this, otherwise any cached references will point to invalid data.
    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void resetRects(bool freeData);

    void markFullIfUsed() { fIsFull = !fDirtyRect.isEmpty(); }
    bool isEmpty() const { return fRectanizer.percentFull() == 0; }
    bool hasAllocation() const { return fData != nullptr; }

    /**
     * Create a clone of this plot. The cloned plot will take the place of the current plot in
     * the atlas
     */
    sk_sp<Plot> clone() const {
        return sk_sp<Plot>(new Plot(
            fPageIndex, fPlotIndex, fGenerationCounter, fX, fY, fWidth, fHeight, fColorType,
            fBytesPerPixel));
    }

#ifdef SK_DEBUG
    void resetListPtrs() {
        fPrev = fNext = nullptr;
        fList = nullptr;
    }
#endif

private:
    ~Plot() override;

    skgpu::AtlasToken fLastUpload;
    skgpu::AtlasToken fLastUse;
    int               fFlushesSinceLastUse;

    struct {
        const uint32_t fPageIndex : 16;
        const uint32_t fPlotIndex : 16;
    };
    AtlasGenerationCounter* const fGenerationCounter;
    uint64_t fGenID;
    PlotLocator fPlotLocator;
    unsigned char* fData;
    const int fWidth;
    const int fHeight;
    const int fX;
    const int fY;
    skgpu::RectanizerSkyline fRectanizer;
    const SkIPoint16 fOffset;  // the offset of the plot in the backing texture
    const SkColorType fColorType;
    const size_t fBytesPerPixel;
    SkIRect fDirtyRect;  // area in the Plot that needs to be uploaded
    bool fIsFull;
    SkDEBUGCODE(bool fDirty;)
};

typedef SkTInternalLList<Plot> PlotList;

} // namespace skgpu

#endif // skgpu_AtlasTypes_DEFINED
