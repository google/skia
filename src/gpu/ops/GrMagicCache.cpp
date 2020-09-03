/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrMagicCache.h"

#include "src/gpu/GrSurfaceProxyView.h"

GrMagicCache::GrMagicCache() {}


GrSurfaceProxyView GrMagicCache::find(const GrUniqueKey&) {
    SkAutoSpinlock lock{fSpinLock};

#if 0
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
#endif
    return {};
}

GrSurfaceProxyView GrMagicCache::add(const GrUniqueKey&, GrSurfaceProxyView) {
    SkAutoSpinlock lock{fSpinLock};
//    this->internalAdd(std::move(blob));
    return {};
}