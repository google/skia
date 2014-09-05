/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMallocPixelRef.h"
#include "SkBitmap.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// assumes ptr was allocated via sk_malloc
static void sk_free_releaseproc(void* ptr, void*) {
    sk_free(ptr);
}

static bool is_valid(const SkImageInfo& info, SkColorTable* ctable) {
    if (info.width() < 0 || info.height() < 0 ||
        (unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType ||
        (unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType)
    {
        return false;
    }

    // these seem like good checks, but currently we have (at least) tests
    // that expect the pixelref to succeed even when there is a mismatch
    // with colortables. fix?
#if 0
    if (kIndex8_SkColorType == info.fColorType && NULL == ctable) {
        return false;
    }
    if (kIndex8_SkColorType != info.fColorType && ctable) {
        return false;
    }
#endif
    return true;
}

SkMallocPixelRef* SkMallocPixelRef::NewDirect(const SkImageInfo& info,
                                              void* addr,
                                              size_t rowBytes,
                                              SkColorTable* ctable) {
    if (!is_valid(info, ctable)) {
        return NULL;
    }
    return SkNEW_ARGS(SkMallocPixelRef,
                      (info, addr, rowBytes, ctable, NULL, NULL));
}


SkMallocPixelRef* SkMallocPixelRef::NewAllocate(const SkImageInfo& info,
                                                size_t requestedRowBytes,
                                                SkColorTable* ctable) {
    if (!is_valid(info, ctable)) {
        return NULL;
    }

    int32_t minRB = SkToS32(info.minRowBytes());
    if (minRB < 0) {
        return NULL;    // allocation will be too large
    }
    if (requestedRowBytes > 0 && (int32_t)requestedRowBytes < minRB) {
        return NULL;    // cannot meet requested rowbytes
    }

    int32_t rowBytes;
    if (requestedRowBytes) {
        rowBytes = SkToS32(requestedRowBytes);
    } else {
        rowBytes = minRB;
    }

    int64_t bigSize = (int64_t)info.height() * rowBytes;
    if (!sk_64_isS32(bigSize)) {
        return NULL;
    }

    size_t size = sk_64_asS32(bigSize);
    SkASSERT(size >= info.getSafeSize(rowBytes));
    void* addr = sk_malloc_flags(size, 0);
    if (NULL == addr) {
        return NULL;
    }

    return SkNEW_ARGS(SkMallocPixelRef,
                      (info, addr, rowBytes, ctable,
                       sk_free_releaseproc, NULL));
}

SkMallocPixelRef* SkMallocPixelRef::NewWithProc(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable,
                                                void* addr,
                                                SkMallocPixelRef::ReleaseProc proc,
                                                void* context) {
    if (!is_valid(info, ctable)) {
        return NULL;
    }
    return SkNEW_ARGS(SkMallocPixelRef,
                      (info, addr, rowBytes, ctable, proc, context));
}

static void sk_data_releaseproc(void*, void* dataPtr) {
    (static_cast<SkData*>(dataPtr))->unref();
}

SkMallocPixelRef* SkMallocPixelRef::NewWithData(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable,
                                                SkData* data) {
    SkASSERT(data != NULL);
    if (!is_valid(info, ctable)) {
        return NULL;
    }
    if ((rowBytes < info.minRowBytes())
        || (data->size() < info.getSafeSize(rowBytes))) {
        return NULL;
    }
    data->ref();
    SkMallocPixelRef* pr
        = SkNEW_ARGS(SkMallocPixelRef,
                     (info, const_cast<void*>(data->data()), rowBytes, ctable,
                      sk_data_releaseproc, static_cast<void*>(data)));
    SkASSERT(pr != NULL);
    // We rely on the immutability of the pixels to make the
    // const_cast okay.
    pr->setImmutable();
    return pr;
}

///////////////////////////////////////////////////////////////////////////////

SkMallocPixelRef::SkMallocPixelRef(const SkImageInfo& info, void* storage,
                                   size_t rowBytes, SkColorTable* ctable,
                                   bool ownsPixels)
    : INHERITED(info)
    , fReleaseProc(ownsPixels ? sk_free_releaseproc : NULL)
    , fReleaseProcContext(NULL) {
    // This constructor is now DEPRICATED.
    SkASSERT(is_valid(info, ctable));
    SkASSERT(rowBytes >= info.minRowBytes());

    if (kIndex_8_SkColorType != info.colorType()) {
        ctable = NULL;
    }

    fStorage = storage;
    fCTable = ctable;
    fRB = rowBytes;
    SkSafeRef(ctable);

    this->setPreLocked(fStorage, rowBytes, fCTable);
}

SkMallocPixelRef::SkMallocPixelRef(const SkImageInfo& info, void* storage,
                                   size_t rowBytes, SkColorTable* ctable,
                                   SkMallocPixelRef::ReleaseProc proc,
                                   void* context)
    : INHERITED(info)
    , fReleaseProc(proc)
    , fReleaseProcContext(context)
{
    SkASSERT(is_valid(info, ctable));
    SkASSERT(rowBytes >= info.minRowBytes());

    if (kIndex_8_SkColorType != info.colorType()) {
        ctable = NULL;
    }

    fStorage = storage;
    fCTable = ctable;
    fRB = rowBytes;
    SkSafeRef(ctable);

    this->setPreLocked(fStorage, rowBytes, fCTable);
}


SkMallocPixelRef::~SkMallocPixelRef() {
    SkSafeUnref(fCTable);
    if (fReleaseProc != NULL) {
        fReleaseProc(fStorage, fReleaseProcContext);
    }
}

bool SkMallocPixelRef::onNewLockPixels(LockRec* rec) {
    rec->fPixels = fStorage;
    rec->fRowBytes = fRB;
    rec->fColorTable = fCTable;
    return true;
}

void SkMallocPixelRef::onUnlockPixels() {
    // nothing to do
}

size_t SkMallocPixelRef::getAllocatedSizeInBytes() const {
    return this->info().getSafeSize(fRB);
}

///////////////////////////////////////////////////////////////////////////////

SkPixelRef* SkMallocPixelRef::PRFactory::create(const SkImageInfo& info, size_t rowBytes,
                                                SkColorTable* ctable) {
    return SkMallocPixelRef::NewAllocate(info, rowBytes, ctable);
}
