
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMallocPixelRef.h"
#include "SkBitmap.h"
#include "SkFlattenable.h"

SkMallocPixelRef::SkMallocPixelRef(void* storage, size_t size,
                                   SkColorTable* ctable) {
    if (NULL == storage) {
        storage = sk_malloc_throw(size);
    }
    fStorage = storage;
    fSize = size;
    fCTable = ctable;
    SkSafeRef(ctable);
}

SkMallocPixelRef::~SkMallocPixelRef() {
    SkSafeUnref(fCTable);
    sk_free(fStorage);
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

    buffer.write32(fSize);
    buffer.writePad(fStorage, fSize);
    if (fCTable) {
        buffer.writeBool(true);
        fCTable->flatten(buffer);
    } else {
        buffer.writeBool(false);
    }
}

SkMallocPixelRef::SkMallocPixelRef(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer, NULL) {
    fSize = buffer.readU32();
    fStorage = sk_malloc_throw(fSize);
    buffer.read(fStorage, fSize);
    if (buffer.readBool()) {
        fCTable = SkNEW_ARGS(SkColorTable, (buffer));
    } else {
        fCTable = NULL;
    }
}

static SkPixelRef::Registrar reg("SkMallocPixelRef",
                                 SkMallocPixelRef::Create);

