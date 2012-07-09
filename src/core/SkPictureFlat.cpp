
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPictureFlat.h"

#include "SkChecksum.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

///////////////////////////////////////////////////////////////////////////////

SkRefCntPlayback::SkRefCntPlayback() : fCount(0), fArray(NULL) {}

SkRefCntPlayback::~SkRefCntPlayback() {
    this->reset(NULL);
}

void SkRefCntPlayback::reset(const SkRefCntSet* rec) {
    for (int i = 0; i < fCount; i++) {
        SkASSERT(fArray[i]);
        fArray[i]->unref();
    }
    SkDELETE_ARRAY(fArray);
    
    if (rec) {
        fCount = rec->count();
        fArray = SkNEW_ARRAY(SkRefCnt*, fCount);
        rec->copyToArray(fArray);
        for (int i = 0; i < fCount; i++) {
            fArray[i]->ref();
        }
    } else {
        fCount = 0;
        fArray = NULL;
    }
}

void SkRefCntPlayback::setCount(int count) {
    this->reset(NULL);
    
    fCount = count;
    fArray = SkNEW_ARRAY(SkRefCnt*, count);
    sk_bzero(fArray, count * sizeof(SkRefCnt*));
}

SkRefCnt* SkRefCntPlayback::set(int index, SkRefCnt* obj) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    SkRefCnt_SafeAssign(fArray[index], obj);
    return obj;
}

///////////////////////////////////////////////////////////////////////////////

SkFlatData* SkFlatData::Create(SkChunkAlloc* heap, const void* obj,
        int index, void (*flattenProc)(SkOrderedWriteBuffer&, const void*),
        SkRefCntSet* refCntRecorder, SkRefCntSet* faceRecorder,
        uint32_t writeBufferflags) {

    // a buffer of 256 bytes should be sufficient for most paints, regions,
    // and matrices.
    intptr_t storage[256];
    SkOrderedWriteBuffer buffer(256, storage, sizeof(storage));
    if (refCntRecorder) {
        buffer.setRefCntRecorder(refCntRecorder);
    }
    if (faceRecorder) {
        buffer.setTypefaceRecorder(faceRecorder);
    }
    buffer.setFlags(writeBufferflags);

    flattenProc(buffer, obj);
    uint32_t size = buffer.size();

    // allocate enough memory to hold both SkFlatData and the serialized
    // contents
    SkFlatData* result = (SkFlatData*) heap->allocThrow(size + sizeof(SkFlatData));
    result->fIndex = index;
    result->fAllocSize = size;

    // put the serialized contents into the data section of the new allocation
    buffer.flatten(result->data());
    result->fChecksum = SkChecksum::Compute(result->data32(), size);
    return result;
}

void SkFlatData::unflatten(void* result,
        void (*unflattenProc)(SkOrderedReadBuffer&, void*),
        SkRefCntPlayback* refCntPlayback,
        SkTypefacePlayback* facePlayback) const {

    SkOrderedReadBuffer buffer(this->data(), fAllocSize);
    if (refCntPlayback) {
        refCntPlayback->setupBuffer(buffer);
    }
    if (facePlayback) {
        facePlayback->setupBuffer(buffer);
    }
    unflattenProc(buffer, result);
    SkASSERT(fAllocSize == SkAlign8((int32_t)buffer.offset()));
}
