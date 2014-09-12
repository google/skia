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
#include "SkStream.h"
#include "SkWriteBuffer.h"

static void sk_inplace_sentinel_releaseproc(const void*, size_t, void*) {
    // we should never get called, as we are just a sentinel
    sk_throw();
}

SkData::SkData(const void* ptr, size_t size, ReleaseProc proc, void* context) {
    fPtr = const_cast<void*>(ptr);
    fSize = size;
    fReleaseProc = proc;
    fReleaseProcContext = context;
}

// This constructor means we are inline with our fPtr's contents. Thus we set fPtr
// to point right after this. We also set our releaseproc to sk_inplace_sentinel_releaseproc,
// since we need to handle "delete" ourselves. See internal_displose().
//
SkData::SkData(size_t size) {
    fPtr = (char*)(this + 1);   // contents are immediately after this
    fSize = size;
    fReleaseProc = sk_inplace_sentinel_releaseproc;
    fReleaseProcContext = NULL;
}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fPtr, fSize, fReleaseProcContext);
    }
}

void SkData::internal_dispose() const {
    if (sk_inplace_sentinel_releaseproc == fReleaseProc) {
        const_cast<SkData*>(this)->fReleaseProc = NULL;    // so we don't call it in our destructor

        this->internal_dispose_restore_refcnt_to_1();
        this->~SkData();        // explicitly call this for refcnt bookkeeping

        sk_free(const_cast<SkData*>(this));
    } else {
        this->internal_dispose_restore_refcnt_to_1();
        SkDELETE(this);
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

SkData* SkData::PrivateNewWithCopy(const void* srcOrNull, size_t length) {
    if (0 == length) {
        return SkData::NewEmpty();
    }
    char* storage = (char*)sk_malloc_throw(sizeof(SkData) + length);
    SkData* data = new (storage) SkData(length);
    if (srcOrNull) {
        memcpy(data->writable_data(), srcOrNull, length);
    }
    return data;
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

SkData* SkData::NewWithCopy(const void* src, size_t length) {
    SkASSERT(src);
    return PrivateNewWithCopy(src, length);
}

SkData* SkData::NewUninitialized(size_t length) {
    return PrivateNewWithCopy(NULL, length);
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

///////////////////////////////////////////////////////////////////////////////

SkData* SkData::NewFromStream(SkStream* stream, size_t size) {
    SkAutoDataUnref data(SkData::NewUninitialized(size));
    if (stream->read(data->writable_data(), size) != size) {
        return NULL;
    }
    return data.detach();
}

