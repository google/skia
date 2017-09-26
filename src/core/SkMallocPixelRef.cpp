/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMallocPixelRef.h"
#include "SkBitmap.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

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


sk_sp<SkPixelRef> SkMallocPixelRef::MakeUsing(void*(*allocProc)(size_t),
                                              const SkImageInfo& info,
                                              size_t rowBytes) {
    if (rowBytes == 0) {
        rowBytes = info.minRowBytes();
    }

    if (!is_valid(info) || !info.validRowBytes(rowBytes)) {
        return nullptr;
    }

    size_t size = 0;
    // if the info is empty, or rowBytes is 0 (which can be valid), then we don't need to compute
    // a size.
    if (!info.isEmpty() && rowBytes > 0) {
#ifdef SK_SUPPORT_LEGACY_SAFESIZE64
        int64_t bigSize = (int64_t)info.height() * rowBytes;
        if (!sk_64_isS32(bigSize)) {
            return nullptr;
        }

        size = sk_64_asS32(bigSize);
        SkASSERT(size >= info.getSafeSize(rowBytes));
#else
        size = info.computeByteSize(rowBytes);
#endif
        if (size == 0) {
            return nullptr; // overflow
        }
    }

    void* addr = allocProc(size);
    if (nullptr == addr) {
        return nullptr;
    }

     return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes,
                                                   sk_free_releaseproc, nullptr));
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeAllocate(const SkImageInfo& info,
                                                size_t rowBytes) {
    auto sk_malloc_nothrow = [](size_t size) { return sk_malloc_flags(size, 0); };
    return MakeUsing(sk_malloc_nothrow, info, rowBytes);
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeZeroed(const SkImageInfo& info,
                                               size_t rowBytes) {
    return MakeUsing(sk_calloc, info, rowBytes);
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
    if (!is_valid(info) || !info.validRowBytes(rowBytes)) {
        return nullptr;
    }
    size_t sizeNeeded = info.computeByteSize(rowBytes);
    if (sizeNeeded == 0 || sizeNeeded > data->size()) {
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
