/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkLazyPtr.h"
#include "SkOSFile.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkData::SkData(const void* ptr, size_t size, ReleaseProc proc, void* context) {
    fPtr = ptr;
    fSize = size;
    fReleaseProc = proc;
    fReleaseProcContext = context;
}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fPtr, fSize, fReleaseProcContext);
    }
}

bool SkData::equals(const SkData* other) const {
    if (NULL == other) {
        return false;
    }

    return fSize == other->fSize && !memcmp(fPtr, other->fPtr, fSize);
}

size_t SkData::copyRange(size_t offset, size_t length, void* buffer) const {
    size_t available = fSize;
    if (offset >= available || 0 == length) {
        return 0;
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    memcpy(buffer, this->bytes() + offset, length);
    return length;
}

///////////////////////////////////////////////////////////////////////////////

SkData* SkData::NewEmptyImpl() {
    return new SkData(NULL, 0, NULL, NULL);
}
void SkData::DeleteEmpty(SkData* ptr) { SkDELETE(ptr); }

SkData* SkData::NewEmpty() {
    SK_DECLARE_STATIC_LAZY_PTR(SkData, empty, NewEmptyImpl, DeleteEmpty);
    return SkRef(empty.get());
}

// assumes fPtr was allocated via sk_malloc
static void sk_free_releaseproc(const void* ptr, size_t, void*) {
    sk_free((void*)ptr);
}

SkData* SkData::NewFromMalloc(const void* data, size_t length) {
    return new SkData(data, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithCopy(const void* data, size_t length) {
    if (0 == length) {
        return SkData::NewEmpty();
    }

    void* copy = sk_malloc_throw(length); // balanced in sk_free_releaseproc
    memcpy(copy, data, length);
    return new SkData(copy, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithProc(const void* data, size_t length,
                            ReleaseProc proc, void* context) {
    return new SkData(data, length, proc, context);
}

// assumes fPtr was allocated with sk_fmmap
static void sk_mmap_releaseproc(const void* addr, size_t length, void*) {
    sk_fmunmap(addr, length);
}

SkData* SkData::NewFromFILE(SkFILE* f) {
    size_t size;
    void* addr = sk_fmmap(f, &size);
    if (NULL == addr) {
        return NULL;
    }

    return SkData::NewWithProc(addr, size, sk_mmap_releaseproc, NULL);
}

SkData* SkData::NewFromFileName(const char path[]) {
    SkFILE* f = path ? sk_fopen(path, kRead_SkFILE_Flag) : NULL;
    if (NULL == f) {
        return NULL;
    }
    SkData* data = NewFromFILE(f);
    sk_fclose(f);
    return data;
}

SkData* SkData::NewFromFD(int fd) {
    size_t size;
    void* addr = sk_fdmmap(fd, &size);
    if (NULL == addr) {
        return NULL;
    }

    return SkData::NewWithProc(addr, size, sk_mmap_releaseproc, NULL);
}

// assumes context is a SkData
static void sk_dataref_releaseproc(const void*, size_t, void* context) {
    SkData* src = reinterpret_cast<SkData*>(context);
    src->unref();
}

SkData* SkData::NewSubset(const SkData* src, size_t offset, size_t length) {
    /*
        We could, if we wanted/need to, just make a deep copy of src's data,
        rather than referencing it. This would duplicate the storage (of the
        subset amount) but would possibly allow src to go out of scope sooner.
     */

    size_t available = src->size();
    if (offset >= available || 0 == length) {
        return SkData::NewEmpty();
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    src->ref(); // this will be balanced in sk_dataref_releaseproc
    return new SkData(src->bytes() + offset, length, sk_dataref_releaseproc,
                         const_cast<SkData*>(src));
}

SkData* SkData::NewWithCString(const char cstr[]) {
    size_t size;
    if (NULL == cstr) {
        cstr = "";
        size = 1;
    } else {
        size = strlen(cstr) + 1;
    }
    return NewWithCopy(cstr, size);
}
