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

static void malloc_freeproc(void* context) {
    sk_free(context);
}

// Makes empty table
SkDataTable::SkDataTable() {
    fCount = 0;
    fElemSize = 0;   // 0 signals that we use fDir instead of fElems
    fU.fDir = NULL;
    fFreeProc = NULL;
    fFreeProcContext = NULL;
}

SkDataTable::SkDataTable(const void* array, size_t elemSize, int count,
                         FreeProc proc, void* context) {
    SkASSERT(count > 0);

    fCount = count;
    fElemSize = elemSize;   // non-zero signals we use fElems instead of fDir
    fU.fElems = (const char*)array;
    fFreeProc = proc;
    fFreeProcContext = context;
}

SkDataTable::SkDataTable(const Dir* dir, int count, FreeProc proc, void* ctx) {
    SkASSERT(count > 0);

    fCount = count;
    fElemSize = 0;  // 0 signals that we use fDir instead of fElems
    fU.fDir = dir;
    fFreeProc = proc;
    fFreeProcContext = ctx;
}

SkDataTable::~SkDataTable() {
    if (fFreeProc) {
        fFreeProc(fFreeProcContext);
    }
}

size_t SkDataTable::atSize(int index) const {
    SkASSERT((unsigned)index < (unsigned)fCount);

    if (fElemSize) {
        return fElemSize;
    } else {
        return fU.fDir[index].fSize;
    }
}

const void* SkDataTable::at(int index, size_t* size) const {
    SkASSERT((unsigned)index < (unsigned)fCount);

    if (fElemSize) {
        if (size) {
            *size = fElemSize;
        }
        return fU.fElems + index * fElemSize;
    } else {
        if (size) {
            *size = fU.fDir[index].fSize;
        }
        return fU.fDir[index].fPtr;
    }
}

SkDataTable::SkDataTable(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fElemSize = 0;
    fU.fElems = NULL;
    fFreeProc = NULL;
    fFreeProcContext = NULL;

    fCount = buffer.read32();
    if (fCount) {
        fElemSize = buffer.read32();
        if (fElemSize) {
            size_t size = buffer.getArrayCount();
            // size is the size of our elems data
            SkASSERT(fCount * fElemSize == size);
            void* addr = sk_malloc_throw(size);
            if (buffer.readByteArray(addr) != size) {
                sk_throw();
            }
            fU.fElems = (const char*)addr;
            fFreeProcContext = addr;
        } else {
            size_t dataSize = buffer.read32();

            size_t allocSize = fCount * sizeof(Dir) + dataSize;
            void* addr = sk_malloc_throw(allocSize);
            Dir* dir = (Dir*)addr;
            char* elem = (char*)(dir + fCount);
            for (int i = 0; i < fCount; ++i) {
                dir[i].fPtr = elem;
                dir[i].fSize = buffer.readByteArray(elem);
                elem += dir[i].fSize;
            }
            fU.fDir = dir;
            fFreeProcContext = addr;
        }
        fFreeProc = malloc_freeproc;
    }
}

void SkDataTable::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.write32(fCount);
    if (fCount) {
        buffer.write32(fElemSize);
        if (fElemSize) {
            buffer.writeByteArray(fU.fElems, fCount * fElemSize);
        } else {
            size_t dataSize = 0;
            for (int i = 0; i < fCount; ++i) {
                dataSize += fU.fDir[i].fSize;
            }
            buffer.write32(dataSize);
            for (int i = 0; i < fCount; ++i) {
                buffer.writeByteArray(fU.fDir[i].fPtr, fU.fDir[i].fSize);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkDataTable* SkDataTable::NewEmpty() {
    static SkDataTable* gEmpty;
    if (NULL == gEmpty) {
        gEmpty = SkNEW(SkDataTable);
    }
    gEmpty->ref();
    return gEmpty;
}

SkDataTable* SkDataTable::NewCopyArrays(const void * const * ptrs,
                                        const size_t sizes[], int count) {
    if (count <= 0) {
        return SkDataTable::NewEmpty();
    }

    size_t dataSize = 0;
    for (int i = 0; i < count; ++i) {
        dataSize += sizes[i];
    }

    size_t bufferSize = count * sizeof(Dir) + dataSize;
    void* buffer = sk_malloc_throw(bufferSize);

    Dir* dir = (Dir*)buffer;
    char* elem = (char*)(dir + count);
    for (int i = 0; i < count; ++i) {
        dir[i].fPtr = elem;
        dir[i].fSize = sizes[i];
        memcpy(elem, ptrs[i], sizes[i]);
        elem += sizes[i];
    }

    return SkNEW_ARGS(SkDataTable, (dir, count, malloc_freeproc, buffer));
}

SkDataTable* SkDataTable::NewCopyArray(const void* array, size_t elemSize,
                                       int count) {
    if (count <= 0) {
        return SkDataTable::NewEmpty();
    }

    size_t bufferSize = elemSize * count;
    void* buffer = sk_malloc_throw(bufferSize);
    memcpy(buffer, array, bufferSize);

    return SkNEW_ARGS(SkDataTable,
                      (buffer, elemSize, count, malloc_freeproc, buffer));
}

SkDataTable* SkDataTable::NewArrayProc(const void* array, size_t elemSize,
                                       int count, FreeProc proc, void* ctx) {
    if (count <= 0) {
        return SkDataTable::NewEmpty();
    }
    return SkNEW_ARGS(SkDataTable, (array, elemSize, count, proc, ctx));
}

///////////////////////////////////////////////////////////////////////////////

static void chunkalloc_freeproc(void* context) {
    SkDELETE((SkChunkAlloc*)context);
}

SkDataTableBuilder::SkDataTableBuilder(size_t minChunkSize)
    : fHeap(NULL)
    , fMinChunkSize(minChunkSize) {}

SkDataTableBuilder::~SkDataTableBuilder() { this->reset(); }

void SkDataTableBuilder::reset(size_t minChunkSize) {
    fMinChunkSize = minChunkSize;
    fDir.reset();
    if (fHeap) {
        SkDELETE(fHeap);
        fHeap = NULL;
    }
}

void SkDataTableBuilder::append(const void* src, size_t size) {
    if (NULL == fHeap) {
        fHeap = SkNEW_ARGS(SkChunkAlloc, (fMinChunkSize));
    }

    void* dst = fHeap->alloc(size, SkChunkAlloc::kThrow_AllocFailType);
    memcpy(dst, src, size);

    SkDataTable::Dir* dir = fDir.append();
    dir->fPtr = dst;
    dir->fSize = size;
}

SkDataTable* SkDataTableBuilder::detachDataTable() {
    const int count = fDir.count();
    if (0 == count) {
        return SkDataTable::NewEmpty();
    }

    // Copy the dir into the heap;
    void* dir = fHeap->alloc(count * sizeof(SkDataTable::Dir),
                             SkChunkAlloc::kThrow_AllocFailType);
    memcpy(dir, fDir.begin(), count * sizeof(SkDataTable::Dir));

    SkDataTable* table = SkNEW_ARGS(SkDataTable,
                                    ((SkDataTable::Dir*)dir, count,
                                     chunkalloc_freeproc, fHeap));
    // we have to detach our fHeap, since we are giving that to the table
    fHeap = NULL;
    fDir.reset();
    return table;
}
