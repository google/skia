/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDataPixelRef.h"
#include "SkData.h"
#include "SkFlattenableBuffers.h"

SkDataPixelRef::SkDataPixelRef(const SkImageInfo& info,
                               SkData* data, size_t rowBytes)
    : INHERITED(info)
    , fData(data)
    , fRB(rowBytes) 
{
    fData->ref();
    this->setPreLocked(const_cast<void*>(fData->data()), rowBytes, NULL);
}

SkDataPixelRef::~SkDataPixelRef() {
    fData->unref();
}

bool SkDataPixelRef::onNewLockPixels(LockRec* rec) {
    rec->fPixels = const_cast<void*>(fData->data());
    rec->fColorTable = NULL;
    rec->fRowBytes = fRB;
    return true;
}

void SkDataPixelRef::onUnlockPixels() {
    // nothing to do
}

size_t SkDataPixelRef::getAllocatedSizeInBytes() const {
    return fData ? fData->size() : 0;
}

void SkDataPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    
    buffer.writeDataAsByteArray(fData);
    buffer.write32(fRB);
}

SkDataPixelRef::SkDataPixelRef(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer, NULL)
{
    fData = buffer.readByteArrayAsData();
    fRB = buffer.read32();
    this->setPreLocked(const_cast<void*>(fData->data()), fRB, NULL);
}
