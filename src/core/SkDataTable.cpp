/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkDataTable.h"
#include "SkFlattenableBuffers.h"

SK_DEFINE_INST_COUNT(SkDataTable)

SkDataTable::SkDataTable(int count, SkData* data)
    : fCount(count)
    , fData(data) {}

SkDataTable::~SkDataTable() {
    fData->unref();
}

struct ElemHead {
    const void* fPtr;
    uintptr_t   fSize;

    static const ElemHead* Get(SkData* data) {
        return (const ElemHead*)(data->data());
    }
};

size_t SkDataTable::atSize(int index) const {
    SkASSERT((unsigned)index < (unsigned)fCount);
    return ElemHead::Get(fData)[index].fSize;
}

const void* SkDataTable::atData(int index, size_t* size) const {
    SkASSERT((unsigned)index < (unsigned)fCount);
    const ElemHead& head = ElemHead::Get(fData)[index];
    if (size) {
        *size = head.fSize;
    }
    return head.fPtr;
}

SkDataTable::SkDataTable(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fCount = buffer.read32();
    fData = buffer.readFlattenableT<SkData>();
}

void SkDataTable::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.write32(fCount);
    buffer.writeFlattenable(fData);
}

///////////////////////////////////////////////////////////////////////////////

SkDataTable* SkDataTable::NewEmpty() {
    static SkDataTable* gEmpty;
    if (NULL == gEmpty) {
        gEmpty = SkNEW_ARGS(SkDataTable, (0, SkData::NewEmpty()));
    }
    gEmpty->ref();
    return gEmpty;
}

SkDataTable* SkDataTable::NewCopyArrays(const void * const * ptrs,
                                        const size_t sizes[], int count) {
    if (count < 0) {
        count = 0;
    }

    size_t headerSize = count * sizeof(ElemHead);
    size_t dataSize = 0;
    for (int i = 0; i < count; ++i) {
        dataSize += sizes[i];
    }

    size_t bufferSize = headerSize + dataSize;
    void* buffer = sk_malloc_throw(bufferSize);

    ElemHead* headerCurr = (ElemHead*)buffer;
    char* dataCurr = (char*)buffer + headerSize;
    for (int i = 0; i < count; ++i) {
        headerCurr[i].fPtr = dataCurr;
        headerCurr[i].fSize = sizes[i];
        memcpy(dataCurr, ptrs[i], sizes[i]);
        dataCurr += sizes[i];
    }

    return SkNEW_ARGS(SkDataTable, (count,
                                    SkData::NewFromMalloc(buffer, bufferSize)));
}

SkDataTable* SkDataTable::NewCopyArray(const void* array, size_t elemSize,
                                       int count) {
    if (count < 0) {
        count = 0;
    }
    
    size_t headerSize = count * sizeof(ElemHead);
    size_t dataSize = count * elemSize;
    
    size_t bufferSize = headerSize + dataSize;
    void* buffer = sk_malloc_throw(bufferSize);
    
    ElemHead* headerCurr = (ElemHead*)buffer;
    char* dataCurr = (char*)buffer + headerSize;
    for (int i = 0; i < count; ++i) {
        headerCurr[i].fPtr = dataCurr;
        headerCurr[i].fSize = elemSize;
        dataCurr += elemSize;
    }
    memcpy((char*)buffer + headerSize, array, dataSize);
    
    return SkNEW_ARGS(SkDataTable, (count,
                                    SkData::NewFromMalloc(buffer, bufferSize)));
}

///////////////////////////////////////////////////////////////////////////////

SkDataTableBuilder::SkDataTableBuilder(size_t minChunkSize)
    : fHeap(minChunkSize) {}

SkDataTableBuilder::~SkDataTableBuilder() {}

void SkDataTableBuilder::reset() {
    fSizes.reset();
    fPtrs.reset();
    fHeap.reset();
}

void SkDataTableBuilder::append(const void* src, size_t size) {
    void* dst = fHeap.alloc(size, SkChunkAlloc::kThrow_AllocFailType);
    memcpy(dst, src, size);

    *fSizes.append() = size;
    *fPtrs.append() = dst;
}

SkDataTable* SkDataTableBuilder::createDataTable() {
    SkASSERT(fSizes.count() == fPtrs.count());
    return SkDataTable::NewCopyArrays(fPtrs.begin(), fSizes.begin(),
                                      fSizes.count());
}

