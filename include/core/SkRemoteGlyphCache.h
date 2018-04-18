/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCache_DEFINED
#define SkRemoteGlyphCache_DEFINED

#include "SkNoDrawCanvas.h"
#include "SkPreConfig.h"
#include "SkScalerContext.h"
#include "SkTextBlobRunIterator.h"
#include "SkWeakRefCnt.h"

class SkStrikeServer;

// A SkTextBlobCacheDiffCanvas is used to populate the SkStrikeServer with ops
// which will be serialized and renderered using the SkStrikeClient.
class SK_API SkTextBlobCacheDiffCanvas : public SkNoDrawCanvas {
public:
    SkTextBlobCacheDiffCanvas(int width, int height, const SkMatrix& deviceMatrix,
                              const SkSurfaceProps& props, SkScalerContextFlags flags,
                              SkStrikeServer* strikeserver);
    ~SkTextBlobCacheDiffCanvas() override;

    SkCanvas::SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec);

protected:
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

    const SkMatrix fDeviceMatrix;
    const SkSurfaceProps fSurfaceProps;
    const SkScalerContextFlags fScalerContextFlags;
    SkStrikeServer* const fStrikeServer;
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

        // Creates a new handle and returns a unique ID that can be used to identify
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

    virtual ~SkStrikeServer() {}

    std::unique_ptr<SkStrikeServer> Create(DiscardableHandleManager* discardableManager);

    // Serializes the typeface to be remoted using this server.
    virtual sk_sp<SkData> serializeTypeface(SkTypeface*) = 0;

    // Serializes the strike data captured using a SkTextBlobCacheDiffCanvas. Any
    // handles locked using the DiscardableHandleManager will be assumed to be
    // unlocked after this call.
    virtual void writeStrikeData(std::vector<uint8_t>* memory);
};

// This class is not thread-safe.
class SK_API SkStrikeClient : public SkWeakRefCnt {
public:
    // An interface to delete handles that may be pinned by the remote server.
    class DiscardableHandleManager {
    public:
        virtual ~DiscardableHandleManager() {}

        // Returns true if the handle was unlocked and can be safely deleted. Once
        // successful, subsequent attempts to delete the same handle are invalid.
        virtual bool deleteHandle(SkDiscardableHandleId) = 0;
    };

    virtual ~SkStrikeClient() {}

    sk_sp<SkStrikeClient> Create(DiscardableHandleManager*);

    // Deserializes the typeface previously serialized using the SkStrikeServer. Returns null if the
    // data is invalid.
    virtual sk_sp<SkTypeface> deserializeTypeface(const void* data, size_t length) = 0;

    // Deserializes the strike data from a SkStrikeServer. All messages generated
    // from a server when serializing the ops must be deserialized before the op
    // is rasterized.
    // Returns false if the data is invalid.
    virtual bool readStrikeData(const void* memory, size_t memorySize) = 0;
};

#endif  // SkRemoteGlyphCache_DEFINED
