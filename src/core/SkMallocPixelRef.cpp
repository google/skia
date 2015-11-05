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
    if (kIndex8_SkColorType == info.fColorType && nullptr == ctable) {
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
        return nullptr;
    }
    return new SkMallocPixelRef(info, addr, rowBytes, ctable, nullptr, nullptr);
}


 SkMallocPixelRef* SkMallocPixelRef::NewUsing(void*(*alloc)(size_t),
                                              const SkImageInfo& info,
                                              size_t requestedRowBytes,
                                              SkColorTable* ctable) {
    if (!is_valid(info, ctable)) {
        return nullptr;
    }

    // only want to permit 31bits of rowBytes
    int64_t minRB = (int64_t)info.minRowBytes64();
    if (minRB < 0 || !sk_64_isS32(minRB)) {
        return nullptr;    // allocation will be too large
    }
    if (requestedRowBytes > 0 && (int32_t)requestedRowBytes < minRB) {
        return nullptr;    // cannot meet requested rowbytes
    }

    int32_t rowBytes;
    if (requestedRowBytes) {
        rowBytes = SkToS32(requestedRowBytes);
    } else {
        rowBytes = minRB;
    }

    int64_t bigSize = (int64_t)info.height() * rowBytes;
    if (!sk_64_isS32(bigSize)) {
        return nullptr;
    }

    size_t size = sk_64_asS32(bigSize);
    SkASSERT(size >= info.getSafeSize(rowBytes));
    void* addr = alloc(size);
    if (nullptr == addr) {
        return nullptr;
    }

    return new SkMallocPixelRef(info, addr, rowBytes, ctable, sk_free_releaseproc, nullptr);
}

SkMallocPixelRef* SkMallocPixelRef::NewAllocate(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable) {
    auto sk_malloc_nothrow = [](size_t size) { return sk_malloc_flags(size, 0); };
    return NewUsing(sk_malloc_nothrow, info, rowBytes, ctable);
}

SkMallocPixelRef* SkMallocPixelRef::NewZeroed(const SkImageInfo& info,
                                              size_t rowBytes,
                                              SkColorTable* ctable) {
    return NewUsing(sk_calloc, info, rowBytes, ctable);
}

SkMallocPixelRef* SkMallocPixelRef::NewWithProc(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable,
                                                void* addr,
                                                SkMallocPixelRef::ReleaseProc proc,
                                                void* context) {
    if (!is_valid(info, ctable)) {
        return nullptr;
    }
    return new SkMallocPixelRef(info, addr, rowBytes, ctable, proc, context);
}

static void sk_data_releaseproc(void*, void* dataPtr) {
    (static_cast<SkData*>(dataPtr))->unref();
}

SkMallocPixelRef* SkMallocPixelRef::NewWithData(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable,
                                                SkData* data) {
    SkASSERT(data != nullptr);
    if (!is_valid(info, ctable)) {
        return nullptr;
    }
    if ((rowBytes < info.minRowBytes())
        || (data->size() < info.getSafeSize(rowBytes))) {
        return nullptr;
    }
    data->ref();
    SkMallocPixelRef* pr =
            new SkMallocPixelRef(info, const_cast<void*>(data->data()), rowBytes, ctable,
                                 sk_data_releaseproc, static_cast<void*>(data));
    SkASSERT(pr != nullptr);
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
    , fReleaseProc(ownsPixels ? sk_free_releaseproc : nullptr)
    , fReleaseProcContext(nullptr) {
    // This constructor is now DEPRICATED.
    SkASSERT(is_valid(info, ctable));
    SkASSERT(rowBytes >= info.minRowBytes());

    if (kIndex_8_SkColorType != info.colorType()) {
        ctable = nullptr;
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
        ctable = nullptr;
    }

    fStorage = storage;
    fCTable = ctable;
    fRB = rowBytes;
    SkSafeRef(ctable);

    this->setPreLocked(fStorage, rowBytes, fCTable);
}


SkMallocPixelRef::~SkMallocPixelRef() {
    SkSafeUnref(fCTable);
    if (fReleaseProc != nullptr) {
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

SkPixelRef* SkMallocPixelRef::ZeroedPRFactory::create(const SkImageInfo& info, size_t rowBytes,
                                                      SkColorTable* ctable) {
    return SkMallocPixelRef::NewZeroed(info, rowBytes, ctable);
}
