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
    if (info.fWidth < 0 ||
        info.fHeight < 0 ||
        (unsigned)info.fColorType > (unsigned)kLastEnum_SkColorType ||
        (unsigned)info.fAlphaType > (unsigned)kLastEnum_SkAlphaType)
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
    if (kIndex8_SkColorType != info.fColorType && NULL != ctable) {
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

    int64_t bigSize = (int64_t)info.fHeight * rowBytes;
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
                                                SkData* data,
                                                size_t offset) {
    SkASSERT(data != NULL);
    SkASSERT(offset <= data->size());
    if (!is_valid(info, ctable)) {
        return NULL;
    }
    if ((rowBytes < info.minRowBytes())
        || ((data->size() - offset) < info.getSafeSize(rowBytes))) {
        return NULL;
    }
    data->ref();
    const void* ptr = static_cast<const void*>(data->bytes() + offset);
    SkMallocPixelRef* pr
        = SkNEW_ARGS(SkMallocPixelRef,
                     (info, const_cast<void*>(ptr), rowBytes, ctable,
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

    if (kIndex_8_SkColorType != info.fColorType) {
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

    if (kIndex_8_SkColorType != info.fColorType) {
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

void SkMallocPixelRef::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.write32(SkToU32(fRB));

    // TODO: replace this bulk write with a chunky one that can trim off any
    // trailing bytes on each scanline (in case rowbytes > width*size)
    size_t size = this->info().getSafeSize(fRB);
    buffer.writeByteArray(fStorage, size);
    buffer.writeBool(fCTable != NULL);
    if (fCTable) {
        fCTable->writeToBuffer(buffer);
    }
}

SkMallocPixelRef::SkMallocPixelRef(SkReadBuffer& buffer)
    : INHERITED(buffer, NULL)
    , fReleaseProc(sk_free_releaseproc)
    , fReleaseProcContext(NULL)
{
    fRB = buffer.read32();
    size_t size = buffer.isValid() ? this->info().getSafeSize(fRB) : 0;
    if (buffer.validateAvailable(size)) {
        fStorage = sk_malloc_throw(size);
        buffer.readByteArray(fStorage, size);
    } else {
        fStorage = NULL;
    }

    if (buffer.readBool()) {
        fCTable = SkNEW_ARGS(SkColorTable, (buffer));
    } else {
        fCTable = NULL;
    }

    this->setPreLocked(fStorage, fRB, fCTable);
}

///////////////////////////////////////////////////////////////////////////////

SkPixelRef* SkMallocPixelRef::PRFactory::create(const SkImageInfo& info,
                                                SkColorTable* ctable) {
    return SkMallocPixelRef::NewAllocate(info, info.minRowBytes(), ctable);
}
