/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobRedrawCoordinator_DEFINED
#define GrTextBlobRedrawCoordinator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/gpu/text/GrTextBlob.h"

#include <functional>

// GrTextBlobRedrawCoordinator reuses data from previous drawing operations using multiple criteria
// to pick the best data for the draw. In addition, it provides a central service for managing
// resource usage through a messageBus.
// The draw data is stored in a three-tiered system. The first tier is keyed by the SkTextBlob's
// uniqueID. The second tier uses the GrTextBlob's key to get a general match for the draw. The
// last tier queries each sub run using canReuse to determine if each sub run can handle the
// drawing parameters.
class GrTextBlobRedrawCoordinator {
public:
    GrTextBlobRedrawCoordinator(uint32_t messageBusID);

    void drawGlyphRunList(const GrClip* clip,
                          const SkMatrixProvider& viewMatrix,
                          const SkGlyphRunList& glyphRunList,
                          const SkPaint& paint,
                          skgpu::v1::SurfaceDrawContext* sdc);

    void freeAll() SK_EXCLUDES(fSpinLock);

    struct PurgeBlobMessage {
        PurgeBlobMessage(uint32_t blobID, uint32_t contextUniqueID)
                : fBlobID(blobID), fContextID(contextUniqueID) {}

        uint32_t fBlobID;
        uint32_t fContextID;
    };

    static void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);

    void purgeStaleBlobs() SK_EXCLUDES(fSpinLock);

    size_t usedBytes() const SK_EXCLUDES(fSpinLock);

    bool isOverBudget() const SK_EXCLUDES(fSpinLock);

private:
    friend class GrTextBlobTestingPeer;
    using TextBlobList = SkTInternalLList<GrTextBlob>;

    struct BlobIDCacheEntry {
        BlobIDCacheEntry();
        explicit BlobIDCacheEntry(uint32_t id);

        static uint32_t GetKey(const BlobIDCacheEntry& entry);

        void addBlob(sk_sp<GrTextBlob> blob);

        void removeBlob(GrTextBlob* blob);

        sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const;

        int findBlobIndex(const GrTextBlob::Key& key) const;

        uint32_t fID;
        // Current clients don't generate multiple GrAtlasTextBlobs per SkTextBlob, so an array w/
        // linear search is acceptable.  If usage changes, we should re-evaluate this structure.
        SkSTArray<1, sk_sp<GrTextBlob>> fBlobs;
    };

    // If not already in the cache, then add it else, return the text blob from the cache.
    sk_sp<GrTextBlob> addOrReturnExisting(
            const SkGlyphRunList& glyphRunList, sk_sp<GrTextBlob> blob) SK_EXCLUDES(fSpinLock);

    sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) SK_EXCLUDES(fSpinLock);

    void remove(GrTextBlob* blob) SK_EXCLUDES(fSpinLock);

    void internalPurgeStaleBlobs() SK_REQUIRES(fSpinLock);

    sk_sp<GrTextBlob> internalAdd(sk_sp<GrTextBlob> blob) SK_REQUIRES(fSpinLock);
    void internalRemove(GrTextBlob* blob) SK_REQUIRES(fSpinLock);

    void internalCheckPurge(GrTextBlob* blob = nullptr) SK_REQUIRES(fSpinLock);

    static const int kDefaultBudget = 1 << 22;

    mutable SkSpinlock fSpinLock;
    TextBlobList fBlobList SK_GUARDED_BY(fSpinLock);
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache SK_GUARDED_BY(fSpinLock);
    size_t fSizeBudget SK_GUARDED_BY(fSpinLock);
    size_t fCurrentSize SK_GUARDED_BY(fSpinLock) {0};

    // In practice 'messageBusID' is always the unique ID of the owning GrContext
    const uint32_t fMessageBusID;
    SkMessageBus<PurgeBlobMessage, uint32_t>::Inbox fPurgeBlobInbox SK_GUARDED_BY(fSpinLock);
};
#endif  // GrTextBlobRedrawCoordinator_DEFINED
