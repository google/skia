/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCachePriv_DEFINED
#define SkRemoteGlyphCachePriv_DEFINED

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "SkData.h"
#include "SkDescriptor.h"
#include "SkDrawLooper.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkRefCnt.h"
#include "SkRemoteGlyphCache.h"
#include "SkSerialProcs.h"
#include "SkTHash.h"
#include "SkTextBlobRunIterator.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"

class Serializer;
class SkGlyphCache;
class SkScalerContextRecDescriptor;
struct WireTypeface;

struct DescHash {
    size_t operator()(const SkDescriptor* key) const { return key->getChecksum(); }
};
struct DescEq {
    bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
        return lhs->getChecksum() == rhs->getChecksum();
    }
};

class SkStrikeServerImpl : public SkStrikeServer {
public:
    class SkGlyphCacheState {
    public:
        SkGlyphCacheState(std::unique_ptr<SkDescriptor> desc,
                          SkDiscardableHandleId discardableHandleId);
        ~SkGlyphCacheState();

        void addGlyph(SkTypeface*, const SkScalerContextEffects&, SkPackedGlyphID);
        void writePendingGlyphs(Serializer* serializer);
        bool has_pending_glyphs() const { return !fPendingGlyphs.empty(); }
        SkDiscardableHandleId discardable_handle_id() const { return fDiscardableHandleId; }

    private:
        // The set of glyphs cached on the remote client.
        SkTHashSet<SkPackedGlyphID> fCachedGlyphs;

        // The set of glyphs which has not yet been serialized and sent to the
        // remote client.
        std::vector<SkPackedGlyphID> fPendingGlyphs;

        std::unique_ptr<SkDescriptor> fDesc;
        const SkDiscardableHandleId fDiscardableHandleId = -1;
        std::unique_ptr<SkScalerContext> fContext;
    };

    SkStrikeServerImpl(DiscardableHandleManager* discardableHandleManager);
    ~SkStrikeServerImpl() override;

    sk_sp<SkData> serializeTypeface(SkTypeface*) override;
    void writeStrikeData(std::vector<uint8_t>* memory) override;

    // Methods used by the SkTextBlobCacheDiffCanvas.
    SkGlyphCacheState* getOrCreateCache(std::unique_ptr<SkDescriptor>);

private:
    using SkGlyphCacheStateMap = std::unordered_map<const SkDescriptor*,
                                                    std::unique_ptr<SkGlyphCacheState>,
                                                    DescHash,
                                                    DescEq>;
    SkGlyphCacheStateMap fRemoteGlyphStateMap;
    DiscardableHandleManager* const fDiscardableHandleManager;
    SkTHashSet<SkFontID> fCachedTypefaces;

    // State cached until the next serialization.
    std::unordered_set<const SkDescriptor*, DescHash, DescEq> fLockedDescs;
    std::vector<WireTypeface> fTypefacesToSend;
};

class SkStrikeClientImpl : public SkStrikeClient {
public:
    class DiscardableHandle {
    public:
        DiscardableHandle();
        DiscardableHandle(SkStrikeClientImpl*, SkDiscardableHandleId);
        ~DiscardableHandle();
        bool deleteHandle();

    private:
        // Note that we keep a weak ref on the client.
        SkStrikeClientImpl* fStrikeClient;
        const SkDiscardableHandleId fDiscardableHandleId;
    };

    SkStrikeClientImpl(DiscardableHandleManager*);
    ~SkStrikeClientImpl() override;

    sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length) override;
    bool readStrikeData(const void* memory, size_t memorySize) override;

    // TODO: Remove these since we don't support pulling this data on-demand.
    void generateFontMetrics(const SkTypefaceProxy& typefaceProxy,
                             const SkScalerContextRec& rec,
                             SkPaint::FontMetrics* metrics);
    void generateMetricsAndImage(const SkTypefaceProxy& typefaceProxy,
                                 const SkScalerContextRec& rec,
                                 SkArenaAlloc* alloc,
                                 SkGlyph* glyph);
    void generatePath(const SkTypefaceProxy& typefaceProxy,
                      const SkScalerContextRec& rec,
                      SkGlyphID glyphID,
                      SkPath* path);

private:
    bool deleteHandle(SkDiscardableHandleId);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fRemoteFontIdToTypeface;
    DiscardableHandleManager* const fDiscardableHandleManager;
};

#endif  // SkRemoteGlyphCachePriv_DEFINED
