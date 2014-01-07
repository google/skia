/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMallocPixelRef.h"
#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"

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
    return SkNEW_ARGS(SkMallocPixelRef, (info, addr, rowBytes, ctable, false));
}

SkMallocPixelRef* SkMallocPixelRef::NewAllocate(const SkImageInfo& info,
                                                size_t requestedRowBytes,
                                                SkColorTable* ctable) {
    if (!is_valid(info, ctable)) {
        return NULL;
    }

    int32_t minRB = info.minRowBytes();
    if (minRB < 0) {
        return NULL;    // allocation will be too large
    }
    if (requestedRowBytes > 0 && (int32_t)requestedRowBytes < minRB) {
        return NULL;    // cannot meet requested rowbytes
    }

    int32_t rowBytes;
    if (requestedRowBytes) {
        rowBytes = requestedRowBytes;
    } else {
        rowBytes = minRB;
    }

    Sk64 bigSize;
    bigSize.setMul(info.fHeight, rowBytes);
    if (!bigSize.is32()) {
        return NULL;
    }

    size_t size = bigSize.get32();
    void* addr = sk_malloc_flags(size, 0);
    if (NULL == addr) {
        return NULL;
    }

    return SkNEW_ARGS(SkMallocPixelRef, (info, addr, rowBytes, ctable, true));
}

///////////////////////////////////////////////////////////////////////////////

SkMallocPixelRef::SkMallocPixelRef(const SkImageInfo& info, void* storage,
                                   size_t rowBytes, SkColorTable* ctable,
                                   bool ownsPixels)
    : INHERITED(info)
    , fOwnPixels(ownsPixels)
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
    
    this->setPreLocked(fStorage, fCTable);
}

SkMallocPixelRef::~SkMallocPixelRef() {
    SkSafeUnref(fCTable);
    if (fOwnPixels) {
        sk_free(fStorage);
    }
}

void* SkMallocPixelRef::onLockPixels(SkColorTable** ctable) {
    *ctable = fCTable;
    return fStorage;
}

void SkMallocPixelRef::onUnlockPixels() {
    // nothing to do
}

size_t SkMallocPixelRef::getAllocatedSizeInBytes() const {
    return this->info().getSafeSize(fRB);
}

void SkMallocPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.write32(fRB);

    // TODO: replace this bulk write with a chunky one that can trim off any
    // trailing bytes on each scanline (in case rowbytes > width*size)
    size_t size = this->info().getSafeSize(fRB);
    buffer.writeByteArray(fStorage, size);
    buffer.writeBool(fCTable != NULL);
    if (fCTable) {
        fCTable->writeToBuffer(buffer);
    }
}

SkMallocPixelRef::SkMallocPixelRef(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer, NULL)
    , fOwnPixels(true)
{
    fRB = buffer.read32();
    size_t size = buffer.isValid() ? this->info().getSafeSize(fRB) : 0;
    fStorage = sk_malloc_throw(size);
    buffer.readByteArray(fStorage, size);
    if (buffer.readBool()) {
        fCTable = SkNEW_ARGS(SkColorTable, (buffer));
    } else {
        fCTable = NULL;
    }

    this->setPreLocked(fStorage, fCTable);
}
