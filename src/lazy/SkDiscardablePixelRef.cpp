/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardablePixelRef.h"
#include "SkDiscardableMemory.h"
#include "SkImageGenerator.h"

SkDiscardablePixelRef::SkDiscardablePixelRef(SkImageGenerator* generator,
                                             const SkImageInfo& info,
                                             size_t rowBytes,
                                             SkDiscardableMemory::Factory* fact)
    : INHERITED(info)
    , fGenerator(generator)
    , fDMFactory(fact)
    , fRowBytes(rowBytes)
    , fDiscardableMemory(NULL)
{
    SkASSERT(fGenerator != NULL);
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

bool SkDiscardablePixelRef::onNewLockPixels(LockRec* rec) {
    if (fDiscardableMemory != NULL) {
        if (fDiscardableMemory->lock()) {
            rec->fPixels = fDiscardableMemory->data();
            rec->fColorTable = NULL;
            rec->fRowBytes = fRowBytes;
            return true;
        }
        SkDELETE(fDiscardableMemory);
        fDiscardableMemory = NULL;
    }
    
    const size_t size = this->info().getSafeSize(fRowBytes);
    if (fDMFactory != NULL) {
        fDiscardableMemory = fDMFactory->create(size);
    } else {
        fDiscardableMemory = SkDiscardableMemory::Create(size);
    }
    if (NULL == fDiscardableMemory) {
        return false;  // Memory allocation failed.
    }

    void* pixels = fDiscardableMemory->data();
    if (!fGenerator->getPixels(this->info(), pixels, fRowBytes)) {
        return false;  // TODO(halcanary) Find out correct thing to do.
    }

    rec->fPixels = pixels;
    rec->fColorTable = NULL;
    rec->fRowBytes = fRowBytes;
    return true;
}

void SkDiscardablePixelRef::onUnlockPixels() {
    if (fDiscardableMemory != NULL) {
        fDiscardableMemory->unlock();
    }
}

bool SkInstallDiscardablePixelRef(SkImageGenerator* generator,
                                  SkBitmap* dst,
                                  SkDiscardableMemory::Factory* factory) {
    SkImageInfo info;
    SkASSERT(generator != NULL);
    if ((NULL == generator)
        || (!generator->getInfo(&info))
        || (!dst->setConfig(info, 0))) {
        SkDELETE(generator);
        return false;
    }
    SkASSERT(dst->config() != SkBitmap::kNo_Config);
    if (dst->empty()) { // Use a normal pixelref.
        SkDELETE(generator);  // Do not need this anymore.
        return dst->allocPixels(NULL, NULL);
    }
    SkAutoTUnref<SkDiscardablePixelRef> ref(SkNEW_ARGS(SkDiscardablePixelRef,
                                                   (generator, info,
                                                    dst->rowBytes(),
                                                    factory)));
    dst->setPixelRef(ref);
    return true;
}
