/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardablePixelRef.h"
#include "SkDiscardableMemory.h"
#include "SkImageGenerator.h"

SkDiscardablePixelRef::SkDiscardablePixelRef(const SkImageInfo& info,
                                             SkImageGenerator* generator,
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
    if (this->isLocked()) {
        fDiscardableMemory->unlock();
    }
    SkDELETE(fDiscardableMemory);
    SkSafeUnref(fDMFactory);
    SkDELETE(fGenerator);
}

bool SkDiscardablePixelRef::onNewLockPixels(LockRec* rec) {
    if (fDiscardableMemory != NULL) {
        if (fDiscardableMemory->lock()) {
            rec->fPixels = fDiscardableMemory->data();
            rec->fColorTable = fCTable.get();
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
    const SkImageInfo& info = this->info();
    SkPMColor colors[256];
    int colorCount = 0;

#ifdef SK_SUPPORT_LEGACY_IMAGEGENERATORAPI
    if (!fGenerator->getPixels(info, pixels, fRowBytes)) {
#else
    if (!fGenerator->getPixels(info, pixels, fRowBytes, colors, &colorCount)) {
#endif
        fDiscardableMemory->unlock();
        SkDELETE(fDiscardableMemory);
        fDiscardableMemory = NULL;
        return false;
    }

    // Note: our ctable is not purgable, as it is not stored in the discardablememory block.
    // This is because SkColorTable is refcntable, and therefore our caller could hold onto it
    // beyond the scope of a lock/unlock. If we change the API/lifecycle for SkColorTable, we
    // could move it into the block, but then again perhaps it is small enough that this doesn't
    // really matter.
    if (colorCount > 0) {
        fCTable.reset(SkNEW_ARGS(SkColorTable, (colors, colorCount)));
    } else {
        fCTable.reset(NULL);
    }

    rec->fPixels = pixels;
    rec->fColorTable = fCTable.get();
    rec->fRowBytes = fRowBytes;
    return true;
}

void SkDiscardablePixelRef::onUnlockPixels() {
    fDiscardableMemory->unlock();
}

bool SkInstallDiscardablePixelRef(SkImageGenerator* generator, SkBitmap* dst,
                                  SkDiscardableMemory::Factory* factory) {
    SkImageInfo info;
    SkAutoTDelete<SkImageGenerator> autoGenerator(generator);
    if ((NULL == autoGenerator.get())
        || (!autoGenerator->getInfo(&info))
        || (!dst->setInfo(info))) {
        return false;
    }
    SkASSERT(dst->colorType() != kUnknown_SkColorType);
    if (dst->empty()) {  // Use a normal pixelref.
        return dst->allocPixels();
    }
    SkAutoTUnref<SkDiscardablePixelRef> ref(
        SkNEW_ARGS(SkDiscardablePixelRef,
                   (info, autoGenerator.detach(), dst->rowBytes(), factory)));
    dst->setPixelRef(ref);
    return true;
}

// This is the public API
bool SkInstallDiscardablePixelRef(SkImageGenerator* generator, SkBitmap* dst) {
    return SkInstallDiscardablePixelRef(generator, dst, NULL);
}
