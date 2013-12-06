/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardablePixelRef.h"
#include "SkDiscardableMemory.h"

SkDiscardablePixelRef::SkDiscardablePixelRef(SkImageGenerator* generator,
                                             const SkImageInfo& info,
                                             size_t size,
                                             size_t rowBytes,
                                             SkDiscardableMemory::Factory* fact)
    : fGenerator(generator)
    , fDMFactory(fact)
    , fInfo(info)
    , fSize(size)
    , fRowBytes(rowBytes)
    , fDiscardableMemory(NULL) {
    SkASSERT(fGenerator != NULL);
    SkASSERT(fSize > 0);
    SkASSERT(fRowBytes > 0);
    // The SkImageGenerator contract requires fGenerator to always
    // decode the same image on each call to getPixels().
    this->setImmutable();
    SkSafeRef(fDMFactory);
}

SkDiscardablePixelRef::~SkDiscardablePixelRef() {
    SkDELETE(fDiscardableMemory);
    SkSafeUnref(fDMFactory);
    SkDELETE(fGenerator);
}

void* SkDiscardablePixelRef::onLockPixels(SkColorTable**) {
    if (fDiscardableMemory != NULL) {
        if (fDiscardableMemory->lock()) {
            return fDiscardableMemory->data();
        }
        SkDELETE(fDiscardableMemory);
        fDiscardableMemory = NULL;
    }
    if (fDMFactory != NULL) {
        fDiscardableMemory = fDMFactory->create(fSize);
    } else {
        fDiscardableMemory = SkDiscardableMemory::Create(fSize);
    }
    if (NULL == fDiscardableMemory) {
        return NULL;  // Memory allocation failed.
    }
    void* pixels = fDiscardableMemory->data();
    if (!fGenerator->getPixels(fInfo, pixels, fRowBytes)) {
        return NULL;  // TODO(halcanary) Find out correct thing to do.
    }
    return pixels;
}
void SkDiscardablePixelRef::onUnlockPixels() {
    if (fDiscardableMemory != NULL) {
        fDiscardableMemory->unlock();
    }
}

bool SkDiscardablePixelRef::Install(SkImageGenerator* generator,
                                    SkBitmap* dst,
                                    SkDiscardableMemory::Factory* factory) {
    SkImageInfo info;
    SkASSERT(generator != NULL);
    if ((NULL == generator)
        || (!generator->getInfo(&info))
        || (!dst->setConfig(info, 0))
        || (0 == dst->getSize())) {  // dst->getSize=0 Probably a bad config
        SkDELETE(generator);
        return false;
    }
    SkAutoTUnref<SkDiscardablePixelRef> ref(SkNEW_ARGS(SkDiscardablePixelRef,
                                                   (generator, info,
                                                    dst->getSize(),
                                                    dst->rowBytes(),
                                                    factory)));
    dst->setPixelRef(ref);
    return true;
}
