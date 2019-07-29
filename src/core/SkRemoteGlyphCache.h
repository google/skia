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

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTHash.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkDevice.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkStrikeInterface.h"
#include "src/core/SkTLazy.h"

class Deserializer;
class Serializer;
enum SkAxisAlignment : uint32_t;
class SkDescriptor;
class SkStrike;
struct SkPackedGlyphID;
enum SkScalerContextFlags : uint32_t;
class SkStrikeCache;
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

        bool fContextSupportsDistanceFieldText = true;
        SkScalar fMinDistanceFieldFontSize = -1.f;
        SkScalar fMaxDistanceFieldFontSize = -1.f;
        int fMaxTextureSize = 0;
        size_t fMaxTextureBytes = 0u;
    };

    SkTextBlobCacheDiffCanvas(int width, int height, const SkSurfaceProps& props,
                              SkStrikeServer* strikeServer, Settings settings = Settings());

    SkTextBlobCacheDiffCanvas(int width, int height, const SkSurfaceProps& props,
                              SkStrikeServer* strikeServer, sk_sp<SkColorSpace> colorSpace,
                              Settings settings = Settings());

    ~SkTextBlobCacheDiffCanvas() override;

protected:
    SkCanvas::SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;
    bool onDoSaveBehind(const SkRect*) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

private:
    class TrackLayerDevice;

    static SkScalar SetupForPath(SkPaint* paint, SkFont* font);
};

using SkDiscardableHandleId = uint32_t;

// This class is not thread-safe.
class SK_API SkStrikeServer final : public SkStrikeCacheInterface {
public:
    // An interface used by the server to create handles for pinning SkStrike
    // entries on the remote client.
    class SK_API DiscardableHandleManager {
    public:
        virtual ~DiscardableHandleManager() = default;

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

        // Returns true if a handle has been deleted on the remote client. It is
        // invalid to use a handle id again with this manager once this returns true.
        // TODO(khushalsagar): Make pure virtual once chrome implementation lands.
        virtual bool isHandleDeleted(SkDiscardableHandleId) { return false; }
    };

    explicit SkStrikeServer(DiscardableHandleManager* discardableHandleManager);
    ~SkStrikeServer() override;

    // Serializes the typeface to be remoted using this server.
    sk_sp<SkData> serializeTypeface(SkTypeface*);

    // Serializes the strike data captured using a SkTextBlobCacheDiffCanvas. Any
    // handles locked using the DiscardableHandleManager will be assumed to be
    // unlocked after this call.
    void writeStrikeData(std::vector<uint8_t>* memory);

    // Methods used internally in skia ------------------------------------------
    class SkGlyphCacheState;

    SkGlyphCacheState* getOrCreateCache(const SkPaint&,
                                        const SkFont& font,
                                        const SkSurfaceProps&,
                                        const SkMatrix&,
                                        SkScalerContextFlags flags,
                                        SkScalerContextEffects* effects);

    SkScopedStrike findOrCreateScopedStrike(const SkDescriptor& desc,
                                            const SkScalerContextEffects& effects,
                                            const SkTypeface& typeface) override;

    void setMaxEntriesInDescriptorMapForTesting(size_t count) {
        fMaxEntriesInDescriptorMap = count;
    }
    size_t remoteGlyphStateMapSizeForTesting() const { return fRemoteGlyphStateMap.size(); }

private:
    static constexpr size_t kMaxEntriesInDescriptorMap = 2000u;

    void checkForDeletedEntries();

    SkGlyphCacheState* getOrCreateCache(const SkDescriptor& desc,
                                        const SkTypeface& typeface,
                                        SkScalerContextEffects effects);

    SkDescriptorMap<std::unique_ptr<SkGlyphCacheState>> fRemoteGlyphStateMap;
    DiscardableHandleManager* const fDiscardableHandleManager;
    SkTHashSet<SkFontID> fCachedTypefaces;
    size_t fMaxEntriesInDescriptorMap = kMaxEntriesInDescriptorMap;

    // Cached serialized typefaces.
    SkTHashMap<SkFontID, sk_sp<SkData>> fSerializedTypefaces;

    // State cached until the next serialization.
    SkDescriptorSet fLockedDescs;
    std::vector<WireTypeface> fTypefacesToSend;
};

class SK_API SkStrikeClient {
public:
    // This enum is used in histogram reporting in chromium. Please don't re-order the list of
    // entries, and consider it to be append-only.
    enum CacheMissType : uint32_t {
        // Hard failures where no fallback could be found.
        kFontMetrics = 0,
        kGlyphMetrics = 1,
        kGlyphImage = 2,
        kGlyphPath = 3,

        // The original glyph could not be found and a fallback was used.
        kGlyphMetricsFallback = 4,
        kGlyphPathFallback = 5,

        kLast = kGlyphPathFallback
    };

    // An interface to delete handles that may be pinned by the remote server.
    class DiscardableHandleManager : public SkRefCnt {
    public:
        virtual ~DiscardableHandleManager() = default;

        // Returns true if the handle was unlocked and can be safely deleted. Once
        // successful, subsequent attempts to delete the same handle are invalid.
        virtual bool deleteHandle(SkDiscardableHandleId) = 0;

        virtual void notifyCacheMiss(CacheMissType) {}

        struct ReadFailureData {
            size_t memorySize;
            size_t bytesRead;
            uint64_t typefaceSize;
            uint64_t strikeCount;
            uint64_t glyphImagesCount;
            uint64_t glyphPathsCount;
        };
        virtual void notifyReadFailure(const ReadFailureData& data) {}
    };

    explicit SkStrikeClient(sk_sp<DiscardableHandleManager>,
                            bool isLogging = true,
                            SkStrikeCache* strikeCache = nullptr);
    ~SkStrikeClient();

    // Deserializes the typeface previously serialized using the SkStrikeServer. Returns null if the
    // data is invalid.
    sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length);

    static bool ReadGlyph(SkTLazy<SkGlyph>& glyph, Deserializer* deserializer);

    // Deserializes the strike data from a SkStrikeServer. All messages generated
    // from a server when serializing the ops must be deserialized before the op
    // is rasterized.
    // Returns false if the data is invalid.
    bool readStrikeData(const volatile void* memory, size_t memorySize);

private:
    class DiscardableStrikePinner;

    sk_sp<SkTypeface> addTypeface(const WireTypeface& wire);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fRemoteFontIdToTypeface;
    sk_sp<DiscardableHandleManager> fDiscardableHandleManager;
    SkStrikeCache* const fStrikeCache;
    const bool fIsLogging;
};

#endif  // SkRemoteGlyphCache_DEFINED
