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
    fReleaseProc = NULL;
    fReleaseProcContext = NULL;
}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fPtr, fReleaseProcContext);
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

    const size_t actualLength = length + sizeof(SkData);
    if (actualLength < length) {
        // we overflowed
        sk_throw();
    }

    char* storage = (char*)sk_malloc_throw(actualLength);
    SkData* data = new (storage) SkData(length);
    if (srcOrNull) {
        memcpy(data->writable_data(), srcOrNull, length);
    }
    return data;
}

///////////////////////////////////////////////////////////////////////////////

// As a template argument these must have external linkage.
SkData* sk_new_empty_data() { return new SkData(NULL, 0, NULL, NULL); }
namespace { void sk_unref_data(SkData* ptr) { return SkSafeUnref(ptr); } }

SK_DECLARE_STATIC_LAZY_PTR(SkData, empty, sk_new_empty_data, sk_unref_data);

SkData* SkData::NewEmpty() {
    return SkRef(empty.get());
}

// assumes fPtr was allocated via sk_malloc
static void sk_free_releaseproc(const void* ptr, void*) {
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

SkData* SkData::NewWithProc(const void* ptr, size_t length, ReleaseProc proc, void* context) {
    return new SkData(ptr, length, proc, context);
}

// assumes fPtr was allocated with sk_fmmap
static void sk_mmap_releaseproc(const void* addr, void* ctx) {
    size_t length = reinterpret_cast<size_t>(ctx);
    sk_fmunmap(addr, length);
}

SkData* SkData::NewFromFILE(SkFILE* f) {
    size_t size;
    void* addr = sk_fmmap(f, &size);
    if (NULL == addr) {
        return NULL;
    }

    return SkData::NewWithProc(addr, size, sk_mmap_releaseproc, reinterpret_cast<void*>(size));
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
static void sk_dataref_releaseproc(const void*, void* context) {
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

