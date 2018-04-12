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

#include "SkData.h"
#include "SkDescriptor.h"
#include "SkDrawLooper.h"
#include "SkGlyphCache.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSerialProcs.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"

class Serializer;
class SkGlyphCache;
class SkScalerContextRecDescriptor;

using DiscardableHandleId = uint32_t;

// An interface to create handles for pinning SkGlyphCache entries on the
// client.
class SK_API SkDiscardableHandleServer {
 public:
  virtual ~SkDiscardableHandleServer() {}

  // Creates a new handle and returns a unique ID that can be used to identify
  // it on the remote client.
  virtual DiscardableHandleId CreateHandle() = 0;

  // Returns true if the handle could be successfully locked. The server can
  // assume it will remain locked until the next set of serialized entries is
  // pulled from the SkStrikeServer.
  // If returns false, the cache entry mapped to the handle has been deleted on
  // the client. Any subsequent attempts to lock the same handle are not
  // allowed.
  virtual bool LockHandle(DiscardableHandleId) = 0;

  // TODO: Add an API to check if something on the other end is deleted without
  // locking, so we can restrict how many handled we are tracking.
  // virtual bool IsHandleValid(uint32_t) = 0;
};

class SK_API SkDiscardableHandleClient {
 public:
  virtual ~SkDiscardableHandleClient() {}

  // Returns true if the handle was unlocked and can be safely deleted. Once
  // successful, subsequent attempts to delete the same handle are invalid.
  virtual bool DeleteHandle(DiscardableHandleId) = 0;
};

struct DescHash {
    size_t operator()(const SkDescriptor* key) const {
        return key->getChecksum();
    }
};
struct DescEq {
    bool operator()(const SkDescriptor* lhs, const SkDescriptor* rhs) const {
        return lhs->getChecksum() == rhs->getChecksum();
    }
};

class SkStrikeServer;

class SK_API SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {
 public:
  SkTextBlobCacheDiffCanvas(
        int width, int height,
        const SkMatrix& deviceMatrix,
        const SkSurfaceProps& props,
        SkScalerContextFlags flags,
        SkStrikeServer* strike_server);
  ~SkTextBlobCacheDiffCanvas() override;

     SkCanvas::SaveLayerStrategy getSaveLayerStrategy(
    const SaveLayerRec&rec);

 protected:
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
SkStrikeServer* const fStrikeServer;
};

struct WireTypeface;

// This class is not thread-safe.
class SK_API SkStrikeServer {
 public:
  class SkGlyphCacheState {
   public:
    SkGlyphCacheState(
        std::unique_ptr<SkDescriptor> desc,
        uint32_t discardable_handle_id);
    ~SkGlyphCacheState();

    void AddGlyph(SkTypeface* typeface, SkPackedGlyphID);
    void writePendingGlyphs(Serializer* serializer);
    bool has_pending_glyphs() const { return !pending_glyphs_.empty(); }
    DiscardableHandleId discardable_handle_id() const {
      return discardable_handle_id_;
    }

   private:
    // The set of glyphs cached on the remote client.
    SkTHashSet<SkPackedGlyphID> cached_glyphs_;

    // The set of glyphs which has not yet been serialized and sent to the
    // remote client.
    std::vector<SkPackedGlyphID> pending_glyphs_;

    std::unique_ptr<SkDescriptor> desc_;
    const DiscardableHandleId discardable_handle_id_ = -1;
    std::unique_ptr<SkScalerContext> context_;
  };

  SkStrikeServer(SkDiscardableHandleServer* discardable_handle_server);
  ~SkStrikeServer();

  void prepareSerialProcs(SkSerialProcs* procs);
  void writeStrikeData(std::vector<uint8_t>* memory);

    // mostly called internally by Skia
    void generateFontMetrics(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkPaint::FontMetrics*);
    void generateMetricsAndImage(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkArenaAlloc*, SkGlyph*);
    bool generatePath(
        const SkTypefaceProxy&, const SkScalerContextRec&, SkGlyphID glyph, SkPath* path);
    SkTypeface* lookupTypeface(SkFontID id);

  // Methods used by the SkTextBlobCacheDiffCanvas.
  SkGlyphCacheState* GetOrCreateCache(std::unique_ptr<SkDescriptor>);

 private:
  sk_sp<SkData> encodeTypeface(SkTypeface* tf);

  using SkGlyphCacheStateMap =
      std::unordered_map<const SkDescriptor*, std::unique_ptr<SkGlyphCacheState>, DescHash, DescEq>;
  SkGlyphCacheStateMap remote_glyph_state_map_;
  SkDiscardableHandleServer* discardable_handle_server_;
  SkTHashSet<uint32_t> cached_typefaces_;

  std::unordered_set<const SkDescriptor*, DescHash, DescEq> locked_descriptors_;
  std::vector<WireTypeface> typefaces_;
};

// This class is not thread-safe.
class SK_API SkStrikeClient {
 public:
  SkStrikeClient(SkDiscardableHandleClient*);
  ~SkStrikeClient();

  void prepareDeserialProcs(SkDeserialProcs* procs);
  void readStrikeData(sk_sp<SkData> data);
  SkString dump() const;

  bool canPurgeGlyphCache(SkGlyphCache* cache);
  void generateFontMetrics(
          const SkTypefaceProxy& typefaceProxy,
          const SkScalerContextRec& rec,
          SkPaint::FontMetrics* metrics);
  void generateMetricsAndImage(
      const SkTypefaceProxy& typefaceProxy,
      const SkScalerContextRec& rec,
      SkArenaAlloc* alloc,
      SkGlyph* glyph);
  void generatePath(
          const SkTypefaceProxy& typefaceProxy,
          const SkScalerContextRec& rec,
          SkGlyphID glyphID,
          SkPath* path);

 private:
  sk_sp<SkTypeface> decodeTypeface(const void* buf, size_t len);

  SkTHashMap<SkFontID, sk_sp<SkTypeface>> remote_font_id_to_typeface_;
  SkTHashMap<SkScalerContext*, DiscardableHandleId> scaler_to_handle_id_map_;
  SkDiscardableHandleClient* const client_;
};

#endif  // SkRemoteGlyphCache_DEFINED
