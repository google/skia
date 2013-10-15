
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMallocPixelRef.h"
#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"

SkMallocPixelRef::SkMallocPixelRef(void* storage, size_t size,
                                   SkColorTable* ctable, bool ownPixels) {
    if (NULL == storage) {
        SkASSERT(ownPixels);
        storage = sk_malloc_throw(size);
    }
    fStorage = storage;
    fSize = size;
    fCTable = ctable;
    SkSafeRef(ctable);
    fOwnPixels = ownPixels;

    this->setPreLocked(fStorage, fCTable);
}

SkMallocPixelRef::~SkMallocPixelRef() {
    SkSafeUnref(fCTable);
    if (fOwnPixels) {
        sk_free(fStorage);
    }
}

void* SkMallocPixelRef::onLockPixels(SkColorTable** ct) {
    *ct = fCTable;
    return fStorage;
}

void SkMallocPixelRef::onUnlockPixels() {
    // nothing to do
}

void SkMallocPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeByteArray(fStorage, fSize);
    buffer.writeBool(fCTable != NULL);
    if (fCTable) {
        fCTable->writeToBuffer(buffer);
    }
}

SkMallocPixelRef::SkMallocPixelRef(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer, NULL) {
    fSize = buffer.getArrayCount();
    fStorage = sk_malloc_throw(fSize);
    buffer.readByteArray(fStorage);
    if (buffer.readBool()) {
        fCTable = SkNEW_ARGS(SkColorTable, (buffer));
    } else {
        fCTable = NULL;
    }
    fOwnPixels = true;

    this->setPreLocked(fStorage, fCTable);
}
