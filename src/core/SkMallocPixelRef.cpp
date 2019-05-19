/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/private/SkMalloc.h"
#include "src/core/SkSafeMath.h"

void* sk_calloc_throw(size_t count, size_t elemSize) {
    return sk_calloc_throw(SkSafeMath::Mul(count, elemSize));
}

void* sk_malloc_throw(size_t count, size_t elemSize) {
    return sk_malloc_throw(SkSafeMath::Mul(count, elemSize));
}

void* sk_realloc_throw(void* buffer, size_t count, size_t elemSize) {
    return sk_realloc_throw(buffer, SkSafeMath::Mul(count, elemSize));
}

void* sk_malloc_canfail(size_t count, size_t elemSize) {
    return sk_malloc_canfail(SkSafeMath::Mul(count, elemSize));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// assumes ptr was allocated via sk_malloc
static void sk_free_releaseproc(void* ptr, void*) {
    sk_free(ptr);
}

static bool is_valid(const SkImageInfo& info) {
    if (info.width() < 0 || info.height() < 0 ||
        (unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType ||
        (unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType)
    {
        return false;
    }
    return true;
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeDirect(const SkImageInfo& info,
                                               void* addr,
                                               size_t rowBytes) {
    if (!is_valid(info)) {
        return nullptr;
    }
    return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes, nullptr, nullptr));
}


sk_sp<SkPixelRef> SkMallocPixelRef::MakeAllocate(const SkImageInfo& info, size_t rowBytes) {
    if (rowBytes == 0) {
        rowBytes = info.minRowBytes();
        // rowBytes can still be zero, if it overflowed (width * bytesPerPixel > size_t)
        // or if colortype is unknown
    }
    if (!is_valid(info) || !info.validRowBytes(rowBytes)) {
        return nullptr;
    }
    size_t size = 0;
    if (!info.isEmpty() && rowBytes) {
        size = info.computeByteSize(rowBytes);
        if (SkImageInfo::ByteSizeOverflowed(size)) {
            return nullptr;
        }
    }
    void* addr = sk_calloc_canfail(size);
    if (nullptr == addr) {
        return nullptr;
    }

    return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes,
                                                  sk_free_releaseproc, nullptr));
}

static void sk_data_releaseproc(void*, void* dataPtr) {
    (static_cast<SkData*>(dataPtr))->unref();
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeWithProc(const SkImageInfo& info,
                                                 size_t rowBytes,
                                                 void* addr,
                                                 SkMallocPixelRef::ReleaseProc proc,
                                                 void* context) {
    if (!is_valid(info)) {
        if (proc) {
            proc(addr, context);
        }
        return nullptr;
    }
    return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes, proc, context));
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeWithData(const SkImageInfo& info,
                                                 size_t rowBytes,
                                                 sk_sp<SkData> data) {
    SkASSERT(data != nullptr);
    if (!is_valid(info)) {
        return nullptr;
    }
    // TODO: what should we return if computeByteSize returns 0?
    // - the info was empty?
    // - we overflowed computing the size?
    if ((rowBytes < info.minRowBytes()) || (data->size() < info.computeByteSize(rowBytes))) {
        return nullptr;
    }
    // must get this address before we call release
    void* pixels = const_cast<void*>(data->data());
    SkPixelRef* pr = new SkMallocPixelRef(info, pixels, rowBytes,
                                          sk_data_releaseproc, data.release());
    pr->setImmutable(); // since we were created with (immutable) data
    return sk_sp<SkPixelRef>(pr);
}

///////////////////////////////////////////////////////////////////////////////

SkMallocPixelRef::SkMallocPixelRef(const SkImageInfo& info, void* storage,
                                   size_t rowBytes,
                                   SkMallocPixelRef::ReleaseProc proc,
                                   void* context)
    : INHERITED(info.width(), info.height(), storage, rowBytes)
    , fReleaseProc(proc)
    , fReleaseProcContext(context)
{}


SkMallocPixelRef::~SkMallocPixelRef() {
    if (fReleaseProc != nullptr) {
        fReleaseProc(this->pixels(), fReleaseProcContext);
    }
}
