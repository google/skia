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
    , fDiscardableMemory(nullptr)
    , fDiscardableMemoryIsLocked(false)
{
    SkASSERT(fGenerator != nullptr);
    SkASSERT(fRowBytes > 0);
    // The SkImageGenerator contract requires fGenerator to always
    // decode the same image on each call to getPixels().
    this->setImmutable();
    SkSafeRef(fDMFactory);
}

SkDiscardablePixelRef::~SkDiscardablePixelRef() {
    if (fDiscardableMemoryIsLocked) {
        fDiscardableMemory->unlock();
        fDiscardableMemoryIsLocked = false;
    }
    delete fDiscardableMemory;
    SkSafeUnref(fDMFactory);
    delete fGenerator;
}

bool SkDiscardablePixelRef::onNewLockPixels(LockRec* rec) {
    if (fDiscardableMemory != nullptr) {
        if (fDiscardableMemory->lock()) {
            fDiscardableMemoryIsLocked = true;
            rec->fPixels = fDiscardableMemory->data();
            rec->fColorTable = fCTable.get();
            rec->fRowBytes = fRowBytes;
            return true;
        }
        delete fDiscardableMemory;
        fDiscardableMemory = nullptr;
        fDiscardableMemoryIsLocked = false;
    }

    const size_t size = this->info().getSafeSize(fRowBytes);

    if (fDMFactory != nullptr) {
        fDiscardableMemory = fDMFactory->create(size);
        fDiscardableMemoryIsLocked = true;
    } else {
        fDiscardableMemory = SkDiscardableMemory::Create(size);
        fDiscardableMemoryIsLocked = true;
    }
    if (nullptr == fDiscardableMemory) {
        fDiscardableMemoryIsLocked = false;
        return false;  // Memory allocation failed.
    }

    void* pixels = fDiscardableMemory->data();
    const SkImageInfo& info = this->info();
    SkPMColor colors[256];
    int colorCount = 0;

    if (!fGenerator->getPixels(info, pixels, fRowBytes, colors, &colorCount)) {
        fDiscardableMemory->unlock();
        fDiscardableMemoryIsLocked = false;
        delete fDiscardableMemory;
        fDiscardableMemory = nullptr;
        return false;
    }

    // Note: our ctable is not purgeable, as it is not stored in the discardablememory block.
    // This is because SkColorTable is refcntable, and therefore our caller could hold onto it
    // beyond the scope of a lock/unlock. If we change the API/lifecycle for SkColorTable, we
    // could move it into the block, but then again perhaps it is small enough that this doesn't
    // really matter.
    if (colorCount > 0) {
        fCTable.reset(new SkColorTable(colors, colorCount));
    } else {
        fCTable.reset(nullptr);
    }

    rec->fPixels = pixels;
    rec->fColorTable = fCTable.get();
    rec->fRowBytes = fRowBytes;
    return true;
}

void SkDiscardablePixelRef::onUnlockPixels() {
    fDiscardableMemory->unlock();
    fDiscardableMemoryIsLocked = false;
}

bool SkDEPRECATED_InstallDiscardablePixelRef(SkImageGenerator* generator, const SkIRect* subset,
                                             SkBitmap* dst, SkDiscardableMemory::Factory* factory) {
    SkAutoTDelete<SkImageGenerator> autoGenerator(generator);
    if (nullptr == autoGenerator.get()) {
        return false;
    }

    SkImageInfo prInfo = autoGenerator->getInfo();
    if (prInfo.isEmpty()) {
        return false;
    }

    SkIPoint origin = SkIPoint::Make(0, 0);
    SkImageInfo bmInfo = prInfo;
    if (subset) {
        const SkIRect prBounds = SkIRect::MakeWH(prInfo.width(), prInfo.height());
        if (subset->isEmpty() || !prBounds.contains(*subset)) {
            return false;
        }
        bmInfo = prInfo.makeWH(subset->width(), subset->height());
        origin.set(subset->x(), subset->y());
    }

    // must compute our desired rowBytes w.r.t. the pixelRef's dimensions, not ours, which may be
    // smaller.
    if (!dst->setInfo(bmInfo, prInfo.minRowBytes())) {
        return false;
    }

    // Since dst->setInfo() may have changed/fixed-up info, we check from the bitmap
    SkASSERT(dst->info().colorType() != kUnknown_SkColorType);

    if (dst->empty()) {  // Use a normal pixelref.
        return dst->tryAllocPixels();
    }
    SkAutoTUnref<SkDiscardablePixelRef> ref(
            new SkDiscardablePixelRef(prInfo, autoGenerator.release(), dst->rowBytes(), factory));
    dst->setPixelRef(ref, origin.x(), origin.y());
    return true;
}

// These are the public API

bool SkDEPRECATED_InstallDiscardablePixelRef(SkImageGenerator* generator, SkBitmap* dst) {
    return SkDEPRECATED_InstallDiscardablePixelRef(generator, nullptr, dst, nullptr);
}

bool SkDEPRECATED_InstallDiscardablePixelRef(SkData* encoded, SkBitmap* dst) {
    SkImageGenerator* generator = SkImageGenerator::NewFromEncoded(encoded);
    return generator ?
            SkDEPRECATED_InstallDiscardablePixelRef(generator, nullptr, dst, nullptr) : false;
}
