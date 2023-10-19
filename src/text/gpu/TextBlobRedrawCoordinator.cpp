/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "src/core/SkDevice.h"
#include "src/core/SkStrikeCache.h"
#include "src/text/GlyphRun.h"

#include <utility>

class SkCanvas;
class SkPaint;

using namespace skia_private;

// This needs to be outside the namespace so we can declare SkMessageBus properly
DECLARE_SKMESSAGEBUS_MESSAGE(sktext::gpu::TextBlobRedrawCoordinator::PurgeBlobMessage,
                             uint32_t, true)
namespace sktext::gpu {
// This function is captured by the above macro using implementations from SkMessageBus.h
static inline bool SkShouldPostMessageToBus(
        const TextBlobRedrawCoordinator::PurgeBlobMessage& msg, uint32_t msgBusUniqueID) {
    return msg.fContextID == msgBusUniqueID;
}

TextBlobRedrawCoordinator::TextBlobRedrawCoordinator(uint32_t messageBusID)
        : fSizeBudget(kDefaultBudget)
        , fMessageBusID(messageBusID)
        , fPurgeBlobInbox(messageBusID) { }

void TextBlobRedrawCoordinator::drawGlyphRunList(SkCanvas* canvas,
                                                 const SkMatrix& viewMatrix,
                                                 const sktext::GlyphRunList& glyphRunList,
                                                 const SkPaint& paint,
                                                 SkStrikeDeviceInfo strikeDeviceInfo,
                                                 const AtlasDrawDelegate& atlasDelegate) {
    sk_sp<TextBlob> blob = this->findOrCreateBlob(viewMatrix, glyphRunList, paint,
                                                  strikeDeviceInfo);

    blob->draw(canvas, glyphRunList.origin(), paint, atlasDelegate);
}

sk_sp<TextBlob> TextBlobRedrawCoordinator::findOrCreateBlob(const SkMatrix& viewMatrix,
                                                            const GlyphRunList& glyphRunList,
                                                            const SkPaint& paint,
                                                            SkStrikeDeviceInfo strikeDeviceInfo) {
    SkMatrix positionMatrix{viewMatrix};
    positionMatrix.preTranslate(glyphRunList.origin().x(), glyphRunList.origin().y());

    auto [canCache, key] = TextBlob::Key::Make(
            glyphRunList, paint, positionMatrix, strikeDeviceInfo);
    sk_sp<TextBlob> blob;
    if (canCache) {
        blob = this->find(key);
    }

    if (blob == nullptr || !blob->canReuse(paint, positionMatrix)) {
        if (blob != nullptr) {
            // We have to remake the blob because changes may invalidate our masks.
            this->remove(blob.get());
        }

        blob = TextBlob::Make(
                glyphRunList, paint, positionMatrix,
                strikeDeviceInfo, SkStrikeCache::GlobalStrikeCache());

        if (canCache) {
            blob->addKey(key);
            // The blob may already have been created on a different thread. Use the first one
            // that was there.
            blob = this->addOrReturnExisting(glyphRunList, blob);
        }
    }

    return blob;
}

static void post_purge_blob_message(uint32_t blobID, uint32_t cacheID) {
    using PurgeBlobMessage = TextBlobRedrawCoordinator::PurgeBlobMessage;
    SkASSERT(blobID != SK_InvalidGenID);
    SkMessageBus<PurgeBlobMessage, uint32_t>::Post(PurgeBlobMessage(blobID, cacheID));
}

sk_sp<TextBlob> TextBlobRedrawCoordinator::addOrReturnExisting(
        const GlyphRunList& glyphRunList, sk_sp<TextBlob> blob) {
    SkAutoSpinlock lock{fSpinLock};
    blob = this->internalAdd(std::move(blob));
    glyphRunList.temporaryShuntBlobNotifyAddedToCache(fMessageBusID, post_purge_blob_message);
    return blob;
}

sk_sp<TextBlob> TextBlobRedrawCoordinator::find(const TextBlob::Key& key) {
    SkAutoSpinlock lock{fSpinLock};
    const BlobIDCacheEntry* idEntry = fBlobIDCache.find(key.fUniqueID);
    if (idEntry == nullptr) {
        return nullptr;
    }

    sk_sp<TextBlob> blob = idEntry->find(key);
    TextBlob* blobPtr = blob.get();
    if (blobPtr != nullptr && blobPtr != fBlobList.head()) {
        fBlobList.remove(blobPtr);
        fBlobList.addToHead(blobPtr);
    }
    return blob;
}

void TextBlobRedrawCoordinator::remove(TextBlob* blob) {
    SkAutoSpinlock lock{fSpinLock};
    this->internalRemove(blob);
}

void TextBlobRedrawCoordinator::internalRemove(TextBlob* blob) {
    auto  id      = blob->key().fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);

    if (idEntry != nullptr) {
        sk_sp<TextBlob> stillExists = idEntry->find(blob->key());
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

void TextBlobRedrawCoordinator::freeAll() {
    SkAutoSpinlock lock{fSpinLock};
    fBlobIDCache.reset();
    fBlobList.reset();
    fCurrentSize = 0;
}

void TextBlobRedrawCoordinator::purgeStaleBlobs() {
    SkAutoSpinlock lock{fSpinLock};
    this->internalPurgeStaleBlobs();
}

void TextBlobRedrawCoordinator::internalPurgeStaleBlobs() {
    TArray<PurgeBlobMessage> msgs;
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

size_t TextBlobRedrawCoordinator::usedBytes() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize;
}

bool TextBlobRedrawCoordinator::isOverBudget() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize > fSizeBudget;
}

void TextBlobRedrawCoordinator::internalCheckPurge(TextBlob* blob) {
    // First, purge all stale blob IDs.
    this->internalPurgeStaleBlobs();

    // If we are still over budget, then unref until we are below budget again
    if (fCurrentSize > fSizeBudget) {
        TextBlobList::Iter iter;
        iter.init(fBlobList, TextBlobList::Iter::kTail_IterStart);
        TextBlob* lruBlob = nullptr;
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

sk_sp<TextBlob> TextBlobRedrawCoordinator::internalAdd(sk_sp<TextBlob> blob) {
    auto  id      = blob->key().fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);
    if (!idEntry) {
        idEntry = fBlobIDCache.set(id, BlobIDCacheEntry(id));
    }

    if (sk_sp<TextBlob> alreadyIn = idEntry->find(blob->key()); alreadyIn) {
        blob = std::move(alreadyIn);
    } else {
        fBlobList.addToHead(blob.get());
        fCurrentSize += blob->size();
        idEntry->addBlob(blob);
    }

    this->internalCheckPurge(blob.get());
    return blob;
}

TextBlobRedrawCoordinator::BlobIDCacheEntry::BlobIDCacheEntry() : fID(SK_InvalidGenID) {}

TextBlobRedrawCoordinator::BlobIDCacheEntry::BlobIDCacheEntry(uint32_t id) : fID(id) {}

uint32_t TextBlobRedrawCoordinator::BlobIDCacheEntry::GetKey(
        const TextBlobRedrawCoordinator::BlobIDCacheEntry& entry) {
    return entry.fID;
}

void TextBlobRedrawCoordinator::BlobIDCacheEntry::addBlob(sk_sp<TextBlob> blob) {
    SkASSERT(blob);
    SkASSERT(blob->key().fUniqueID == fID);
    SkASSERT(!this->find(blob->key()));

    fBlobs.emplace_back(std::move(blob));
}

void TextBlobRedrawCoordinator::BlobIDCacheEntry::removeBlob(TextBlob* blob) {
    SkASSERT(blob);
    SkASSERT(blob->key().fUniqueID == fID);

    auto index = this->findBlobIndex(blob->key());
    SkASSERT(index >= 0);

    fBlobs.removeShuffle(index);
}

sk_sp<TextBlob>
TextBlobRedrawCoordinator::BlobIDCacheEntry::find(const TextBlob::Key& key) const {
    auto index = this->findBlobIndex(key);
    return index < 0 ? nullptr : fBlobs[index];
}

int TextBlobRedrawCoordinator::BlobIDCacheEntry::findBlobIndex(const TextBlob::Key& key) const {
    for (int i = 0; i < fBlobs.size(); ++i) {
        if (fBlobs[i]->key() == key) {
            return i;
        }
    }
    return -1;
}

}  // namespace sktext::gpu
