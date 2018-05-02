/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCache_DEFINED
#define SkRemoteGlyphCache_DEFINED

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "SkData.h"
#include "SkDescriptor.h"
#include "SkDrawLooper.h"
#include "SkGlyphCache.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkRefCnt.h"
#include "SkSerialProcs.h"
#include "SkStrikeCache.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"

// The client uses a SkStrikeCacheClientRPC to send and receive data.
using SkStrikeCacheClientRPC = std::function<sk_sp<SkData>(const SkData&)>;

class SkScalerContextRecDescriptor {
public:
    SkScalerContextRecDescriptor() {}
    explicit SkScalerContextRecDescriptor(const SkScalerContextRec& rec) {
        auto desc = reinterpret_cast<SkDescriptor*>(&fDescriptor);
        desc->init();
        desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
        desc->computeChecksum();
        SkASSERT(sizeof(fDescriptor) == desc->getLength());
    }

    explicit SkScalerContextRecDescriptor(const SkDescriptor& desc)
            : SkScalerContextRecDescriptor(ExtractRec(desc)) { }

    SkScalerContextRecDescriptor& operator=(const SkScalerContextRecDescriptor& rhs) {
        std::memcpy(&fDescriptor, &rhs.fDescriptor, rhs.desc().getLength());
        return *this;
    }

    const SkDescriptor& desc() const {
        return *reinterpret_cast<const SkDescriptor*>(&fDescriptor);
    }

    struct Hash {
        uint32_t operator()(SkScalerContextRecDescriptor const& s) const {
            return s.desc().getChecksum();
        }
    };

    friend bool operator==(const SkScalerContextRecDescriptor& lhs,
                           const SkScalerContextRecDescriptor& rhs ) {
        return lhs.desc() == rhs.desc();
    }

private:
    static SkScalerContextRec ExtractRec(const SkDescriptor& desc) {
        uint32_t size;
        auto recPtr = desc.findEntry(kRec_SkDescriptorTag, &size);

        SkScalerContextRec result;
        std::memcpy(&result, recPtr, size);
        return result;
    }
    // The system only passes descriptors without effects. That is why it uses a fixed size
    // descriptor. storageFor is needed because some of the constructors below are private.
    template <typename T>
    using storageFor = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    struct {
        storageFor<SkDescriptor>        dummy1;
        storageFor<SkDescriptor::Entry> dummy2;
        storageFor<SkScalerContextRec>  dummy3;
    } fDescriptor;
};

class SkStrikeDifferences {
public:
    SkStrikeDifferences(SkFontID typefaceID, std::unique_ptr<SkDescriptor> desc);
    void add(uint16_t glyphID, SkIPoint pos);
    SkFontID fTypefaceID;
    std::unique_ptr<SkDescriptor> fDesc;
    std::unique_ptr<SkTHashSet<SkPackedGlyphID>> fGlyphIDs =
            skstd::make_unique<SkTHashSet<SkPackedGlyphID>>();
};

class SkStrikeCacheDifferenceSpec {
public:
    SkStrikeDifferences& findStrikeDifferences(const SkDescriptor& desc, SkFontID typefaceID);
    int strikeCount() const { return fDescriptorToDifferencesMap.size(); }
    size_t sizeBytes() const;
    template <typename PerStrike, typename PerGlyph>
    void iterateDifferences(PerStrike perStrike, PerGlyph perGlyph) const;

private:
    SkDescriptorMap<SkStrikeDifferences> fDescriptorToDifferencesMap{16};
};

class SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {
public:
    SkTextBlobCacheDiffCanvas(int width, int height,
                              const SkMatrix& deviceMatrix,
                              const SkSurfaceProps& props,
                              SkScalerContextFlags flags,
                              SkStrikeCacheDifferenceSpec* strikeDiffs);

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;

    void onDrawTextBlob(
            const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) override;

private:
    void processLooper(
            const SkPoint& position,
            const SkTextBlobRunIterator& it,
            const SkPaint& origPaint,
            SkDrawLooper* looper);

    void processGlyphRun(
            const SkPoint& position,
            const SkTextBlobRunIterator& it,
            const SkPaint& runPaint);

    const SkMatrix fDeviceMatrix;
    const SkSurfaceProps fSurfaceProps;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeCacheDifferenceSpec* const fStrikeCacheDiff;
};

class SkStrikeServer {
public:
    SkStrikeServer();
    ~SkStrikeServer();

    // embedding clients call these methods
    void serve(const SkData&, std::vector<uint8_t>*);

    void prepareSerializeProcs(SkSerialProcs* procs);

    // mostly called internally by Skia
    SkScalerContext* generateScalerContext(
            const SkScalerContextRecDescriptor& desc, SkFontID typefaceId);

private:
    using DescriptorToContextMap = SkTHashMap<SkScalerContextRecDescriptor,
            std::unique_ptr<SkScalerContext>,
            SkScalerContextRecDescriptor::Hash>;

    sk_sp<SkData> encodeTypeface(SkTypeface* tf);

    int fOpCount = 0;
    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fTypefaceMap;
    DescriptorToContextMap fScalerContextMap;
};

class SkStrikeClient {
public:
    SkStrikeClient(SkStrikeCacheClientRPC);

    // embedding clients call these methods
    void primeStrikeCache(const SkStrikeCacheDifferenceSpec&);
    void prepareDeserializeProcs(SkDeserialProcs* procs);

    // mostly called internally by Skia
    void generateFontMetrics(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkPaint::FontMetrics*);
    void generateMetricsAndImage(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkArenaAlloc*, SkGlyph*);
    bool generatePath(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkGlyphID glyph, SkPath* path);
    SkTypeface* lookupTypeface(SkFontID id);

private:
    sk_sp<SkTypeface> decodeTypeface(const void* buf, size_t len);

    // TODO: Figure out how to manage the entries for the following maps.
    SkTHashMap<SkFontID, sk_sp<SkTypefaceProxy>> fMapIdToTypeface;

    SkStrikeCacheClientRPC fClientRPC;

    std::vector<uint8_t> fBuffer;
};

#endif  // SkRemoteGlyphCache_DEFINED
