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
#include <unordered_set>
#include <vector>

#include "../private/SkTHash.h"
#include "SkData.h"
#include "SkDrawLooper.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkRefCnt.h"
#include "SkRemoteGlyphCache.h"
#include "SkSerialProcs.h"
#include "SkTypeface.h"

class Serializer;
class SkDescriptor;
class SkGlyphCache;
struct SkPackedGlyphID;
enum SkScalerContextFlags : uint32_t;
class SkScalerContextRecDescriptor;
class SkTextBlobRunIterator;
class SkTypefaceProxy;
struct WireTypeface;

class SkStrikeServer;

struct SkDescriptorMapOperators {
    size_t operator()(const SkDescriptor* key) const;
    bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const;
};

template <typename T>
using SkDescriptorMap = std::unordered_map<const SkDescriptor*, T, SkDescriptorMapOperators,
                                           SkDescriptorMapOperators>;

using SkDescriptorSet =
        std::unordered_set<const SkDescriptor*, SkDescriptorMapOperators, SkDescriptorMapOperators>;

// A SkTextBlobCacheDiffCanvas is used to populate the SkStrikeServer with ops
// which will be serialized and renderered using the SkStrikeClient.
class SK_API SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {
public:
    struct SK_API Settings {
        Settings();
        ~Settings();

        bool fContextSupportsDistanceFieldText = true;
        SkScalar fMinDistanceFieldFontSize = -1.f;
        SkScalar fMaxDistanceFieldFontSize = -1.f;
    };
    SkTextBlobCacheDiffCanvas(int width, int height, const SkMatrix& deviceMatrix,
                              const SkSurfaceProps& props, SkStrikeServer* strikeserver,
                              Settings settings = Settings());
    ~SkTextBlobCacheDiffCanvas() override;

protected:
    SkCanvas::SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;

    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

private:
    void processLooper(const SkPoint& position,
                       const SkTextBlobRunIterator& it,
                       const SkPaint& origPaint,
                       SkDrawLooper* looper);
    void processGlyphRun(const SkPoint& position,
                         const SkTextBlobRunIterator& it,
                         const SkPaint& runPaint);
    void processGlyphRunForPaths(const SkTextBlobRunIterator& it, const SkPaint& runPaint);
    void processGlyphRunForDFT(const SkTextBlobRunIterator& it, const SkPaint& runPaint,
                               SkScalerContextFlags flags);

    const SkMatrix fDeviceMatrix;
    const SkSurfaceProps fSurfaceProps;
    SkStrikeServer* const fStrikeServer;
    const Settings fSettings;
};

using SkDiscardableHandleId = uint32_t;

// This class is not thread-safe.
class SK_API SkStrikeServer {
public:
    // An interface used by the server to create handles for pinning SkGlyphCache
    // entries on the remote client.
    class SK_API DiscardableHandleManager {
    public:
        virtual ~DiscardableHandleManager() {}

        // Creates a new *locked* handle and returns a unique ID that can be used to identify
        // it on the remote client.
        virtual SkDiscardableHandleId createHandle() = 0;

        // Returns true if the handle could be successfully locked. The server can
        // assume it will remain locked until the next set of serialized entries is
        // pulled from the SkStrikeServer.
        // If returns false, the cache entry mapped to the handle has been deleted
        // on the client. Any subsequent attempts to lock the same handle are not
        // allowed.
        virtual bool lockHandle(SkDiscardableHandleId) = 0;

        // TODO(khushalsagar): Add an API which checks whether a handle is still
        // valid without locking, so we can avoid tracking stale handles once they
        // have been purged on the remote side.
    };

    SkStrikeServer(DiscardableHandleManager* discardableHandleManager);
    ~SkStrikeServer();

    // Serializes the typeface to be remoted using this server.
    sk_sp<SkData> serializeTypeface(SkTypeface*);

    // Serializes the strike data captured using a SkTextBlobCacheDiffCanvas. Any
    // handles locked using the DiscardableHandleManager will be assumed to be
    // unlocked after this call.
    void writeStrikeData(std::vector<uint8_t>* memory);

    // Methods used internally in skia ------------------------------------------
    class SkGlyphCacheState {
    public:
        SkGlyphCacheState(std::unique_ptr<SkDescriptor> deviceDescriptor,
                          std::unique_ptr<SkDescriptor> keyDescriptor,
                          SkDiscardableHandleId discardableHandleId);
        ~SkGlyphCacheState();

        void addGlyph(SkTypeface*, const SkScalerContextEffects&, SkPackedGlyphID, bool pathOnly);
        void writePendingGlyphs(Serializer* serializer);
        bool has_pending_glyphs() const {
            return !fPendingGlyphImages.empty() || !fPendingGlyphPaths.empty();
        }
        SkDiscardableHandleId discardable_handle_id() const { return fDiscardableHandleId; }
        const SkDescriptor& getDeviceDescriptor() {
            return *fDeviceDescriptor;
        }

        const SkDescriptor& getKeyDescriptor() {
            return *fKeyDescriptor;
        }

    private:
        void writeGlyphPath(const SkPackedGlyphID& glyphID, Serializer* serializer) const;

        // The set of glyphs cached on the remote client.
        SkTHashSet<SkPackedGlyphID> fCachedGlyphImages;
        SkTHashSet<SkPackedGlyphID> fCachedGlyphPaths;

        // The set of glyphs which has not yet been serialized and sent to the
        // remote client.
        std::vector<SkPackedGlyphID> fPendingGlyphImages;
        std::vector<SkPackedGlyphID> fPendingGlyphPaths;

        // The device descriptor is used to create the scaler context. The glyphs to have the
        // correct device rendering. The key descriptor is used for communication. The GPU side will
        // create descriptors with out the device filtering, thus matching the key descriptor.
        std::unique_ptr<SkDescriptor> fDeviceDescriptor;
        std::unique_ptr<SkDescriptor> fKeyDescriptor;
        const SkDiscardableHandleId fDiscardableHandleId = static_cast<SkDiscardableHandleId>(-1);

        // The context built using fDeviceDescriptor
        std::unique_ptr<SkScalerContext> fContext;
    };

    SkGlyphCacheState* getOrCreateCache(const SkPaint&, const SkSurfaceProps*, const SkMatrix*,
                                        SkScalerContextFlags flags, SkScalerContextRec* deviceRec,
                                        SkScalerContextEffects* effects);

private:
    SkDescriptorMap<std::unique_ptr<SkGlyphCacheState>> fRemoteGlyphStateMap;
    DiscardableHandleManager* const fDiscardableHandleManager;
    SkTHashSet<SkFontID> fCachedTypefaces;

    // State cached until the next serialization.
    SkDescriptorSet fLockedDescs;
    std::vector<WireTypeface> fTypefacesToSend;
};

class SK_API SkStrikeClient {
public:
    // An interface to delete handles that may be pinned by the remote server.
    class DiscardableHandleManager : public SkRefCnt {
    public:
        virtual ~DiscardableHandleManager() {}

        // Returns true if the handle was unlocked and can be safely deleted. Once
        // successful, subsequent attempts to delete the same handle are invalid.
        virtual bool deleteHandle(SkDiscardableHandleId) = 0;
    };

    SkStrikeClient(sk_sp<DiscardableHandleManager>);
    ~SkStrikeClient();

    // Deserializes the typeface previously serialized using the SkStrikeServer. Returns null if the
    // data is invalid.
    sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length);

    // Deserializes the strike data from a SkStrikeServer. All messages generated
    // from a server when serializing the ops must be deserialized before the op
    // is rasterized.
    // Returns false if the data is invalid.
    bool readStrikeData(const volatile void* memory, size_t memorySize);

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
    class DiscardableStrikePinner;

    sk_sp<SkTypeface> addTypeface(const WireTypeface& wire);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fRemoteFontIdToTypeface;
    sk_sp<DiscardableHandleManager> fDiscardableHandleManager;
};

#endif  // SkRemoteGlyphCache_DEFINED
