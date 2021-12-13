/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrTextBlobRedrawCoordinator.h"

#include "src/gpu/v1/SurfaceDrawContext_v1.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrTextBlobRedrawCoordinator::PurgeBlobMessage, uint32_t, true)

// This function is captured by the above macro using implementations from SkMessageBus.h
static inline bool SkShouldPostMessageToBus(
        const GrTextBlobRedrawCoordinator::PurgeBlobMessage& msg, uint32_t msgBusUniqueID) {
    return msg.fContextID == msgBusUniqueID;
}

GrTextBlobRedrawCoordinator::GrTextBlobRedrawCoordinator(uint32_t messageBusID)
        : fSizeBudget(kDefaultBudget)
        , fMessageBusID(messageBusID)
        , fPurgeBlobInbox(messageBusID) { }

void GrTextBlobRedrawCoordinator::drawGlyphRunList(const GrClip* clip,
                                                   const SkMatrixProvider& viewMatrix,
                                                   const SkGlyphRunList& glyphRunList,
                                                   const SkPaint& paint,
                                                   skgpu::v1::SurfaceDrawContext* sdc) {

    SkMatrix positionMatrix{viewMatrix.localToDevice()};
    positionMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());

    GrSDFTControl control =
            sdc->recordingContext()->priv().getSDFTControl(
                    sdc->surfaceProps().isUseDeviceIndependentFonts());

    auto [canCache, key] = GrTextBlob::Key::Make(glyphRunList,
                                                 paint,
                                                 sdc->surfaceProps(),
                                                 sdc->colorInfo(),
                                                 positionMatrix,
                                                 control);
    sk_sp<GrTextBlob> blob;
    if (canCache) {
        blob = this->find(key);
    }

    if (blob == nullptr || !blob->canReuse(paint, positionMatrix)) {
        if (blob != nullptr) {
            // We have to remake the blob because changes may invalidate our masks.
            this->remove(blob.get());
        }

        const bool padAtlas =
                sdc->recordingContext()->priv().options().fSupportBilerpFromGlyphAtlas;
        blob = GrTextBlob::Make(
                glyphRunList, paint, positionMatrix, padAtlas, control, sdc->glyphRunPainter());

        if (canCache) {
            blob->addKey(key);
            // The blob may already have been created on a different thread. Use the first one
            // that was there.
            blob = this->addOrReturnExisting(glyphRunList, blob);
        }
    }

    for (const GrSubRun& subRun : blob->subRunList()) {
        subRun.draw(clip, viewMatrix, glyphRunList.origin(), paint, sdc);
    }
}

sk_sp<GrTextBlob> GrTextBlobRedrawCoordinator::addOrReturnExisting(
        const SkGlyphRunList& glyphRunList, sk_sp<GrTextBlob> blob) {
    SkAutoSpinlock lock{fSpinLock};
    blob = this->internalAdd(std::move(blob));
    glyphRunList.temporaryShuntBlobNotifyAddedToCache(fMessageBusID);
    return blob;
}

sk_sp<GrTextBlob> GrTextBlobRedrawCoordinator::find(const GrTextBlob::Key& key) {
    SkAutoSpinlock lock{fSpinLock};
    const BlobIDCacheEntry* idEntry = fBlobIDCache.find(key.fUniqueID);
    if (idEntry == nullptr) {
        return nullptr;
    }

    sk_sp<GrTextBlob> blob = idEntry->find(key);
    GrTextBlob* blobPtr = blob.get();
    if (blobPtr != nullptr && blobPtr != fBlobList.head()) {
        fBlobList.remove(blobPtr);
        fBlobList.addToHead(blobPtr);
    }
    return blob;
}

void GrTextBlobRedrawCoordinator::remove(GrTextBlob* blob) {
    SkAutoSpinlock lock{fSpinLock};
    this->internalRemove(blob);
}

void GrTextBlobRedrawCoordinator::internalRemove(GrTextBlob* blob) {
    auto  id      = blob->key().fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);

    if (idEntry != nullptr) {
        sk_sp<GrTextBlob> stillExists = idEntry->find(blob->key());
        if (blob == stillExists.get())  {
            fCurrentSize -= blob->size();
            fBlobList.remove(blob);
            idEntry->removeBlob(blob);
            if (idEntry->fBlobs.empty()) {
                fBlobIDCache.remove(id);
            }
        }
    }
}

void GrTextBlobRedrawCoordinator::freeAll() {
    SkAutoSpinlock lock{fSpinLock};
    fBlobIDCache.reset();
    fBlobList.reset();
    fCurrentSize = 0;
}

void GrTextBlobRedrawCoordinator::PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID) {
    SkASSERT(blobID != SK_InvalidGenID);
    SkMessageBus<PurgeBlobMessage, uint32_t>::Post(PurgeBlobMessage(blobID, cacheID));
}

void GrTextBlobRedrawCoordinator::purgeStaleBlobs() {
    SkAutoSpinlock lock{fSpinLock};
    this->internalPurgeStaleBlobs();
}

void GrTextBlobRedrawCoordinator::internalPurgeStaleBlobs() {
    SkTArray<PurgeBlobMessage> msgs;
    fPurgeBlobInbox.poll(&msgs);

    for (const auto& msg : msgs) {
        auto* idEntry = fBlobIDCache.find(msg.fBlobID);
        if (!idEntry) {
            // no cache entries for id
            continue;
        }

        // remove all blob entries from the LRU list
        for (const auto& blob : idEntry->fBlobs) {
            fCurrentSize -= blob->size();
            fBlobList.remove(blob.get());
        }

        // drop the idEntry itself (unrefs all blobs)
        fBlobIDCache.remove(msg.fBlobID);
    }
}

size_t GrTextBlobRedrawCoordinator::usedBytes() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize;
}

bool GrTextBlobRedrawCoordinator::isOverBudget() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize > fSizeBudget;
}

void GrTextBlobRedrawCoordinator::internalCheckPurge(GrTextBlob* blob) {
    // First, purge all stale blob IDs.
    this->internalPurgeStaleBlobs();

    // If we are still over budget, then unref until we are below budget again
    if (fCurrentSize > fSizeBudget) {
        TextBlobList::Iter iter;
        iter.init(fBlobList, TextBlobList::Iter::kTail_IterStart);
        GrTextBlob* lruBlob = nullptr;
        while (fCurrentSize > fSizeBudget && (lruBlob = iter.get()) && lruBlob != blob) {
            // Backup the iterator before removing and unrefing the blob
            iter.prev();

            this->internalRemove(lruBlob);
        }

    #ifdef SPEW_BUDGET_MESSAGE
        if (fCurrentSize > fSizeBudget) {
            SkDebugf("Single textblob is larger than our whole budget");
        }
    #endif
    }
}

sk_sp<GrTextBlob> GrTextBlobRedrawCoordinator::internalAdd(sk_sp<GrTextBlob> blob) {
    auto  id      = blob->key().fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);
    if (!idEntry) {
        idEntry = fBlobIDCache.set(id, BlobIDCacheEntry(id));
    }

    if (sk_sp<GrTextBlob> alreadyIn = idEntry->find(blob->key()); alreadyIn) {
        blob = std::move(alreadyIn);
    } else {
        fBlobList.addToHead(blob.get());
        fCurrentSize += blob->size();
        idEntry->addBlob(blob);
    }

    this->internalCheckPurge(blob.get());
    return blob;
}

GrTextBlobRedrawCoordinator::BlobIDCacheEntry::BlobIDCacheEntry() : fID(SK_InvalidGenID) {}

GrTextBlobRedrawCoordinator::BlobIDCacheEntry::BlobIDCacheEntry(uint32_t id) : fID(id) {}

uint32_t GrTextBlobRedrawCoordinator::BlobIDCacheEntry::GetKey(
        const GrTextBlobRedrawCoordinator::BlobIDCacheEntry& entry) {
    return entry.fID;
}

void GrTextBlobRedrawCoordinator::BlobIDCacheEntry::addBlob(sk_sp<GrTextBlob> blob) {
    SkASSERT(blob);
    SkASSERT(blob->key().fUniqueID == fID);
    SkASSERT(!this->find(blob->key()));

    fBlobs.emplace_back(std::move(blob));
}

void GrTextBlobRedrawCoordinator::BlobIDCacheEntry::removeBlob(GrTextBlob* blob) {
    SkASSERT(blob);
    SkASSERT(blob->key().fUniqueID == fID);

    auto index = this->findBlobIndex(blob->key());
    SkASSERT(index >= 0);

    fBlobs.removeShuffle(index);
}

sk_sp<GrTextBlob>
GrTextBlobRedrawCoordinator::BlobIDCacheEntry::find(const GrTextBlob::Key& key) const {
    auto index = this->findBlobIndex(key);
    return index < 0 ? nullptr : fBlobs[index];
}

int GrTextBlobRedrawCoordinator::BlobIDCacheEntry::findBlobIndex(const GrTextBlob::Key& key) const {
    for (int i = 0; i < fBlobs.count(); ++i) {
        if (fBlobs[i]->key() == key) {
            return i;
        }
    }
    return -1;
}
