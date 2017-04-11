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

static bool is_valid(const SkImageInfo& info, SkColorTable* ctable) {
    if (info.width() < 0 || info.height() < 0 ||
        (unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType ||
        (unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType)
    {
        return false;
    }

    // these seem like good checks, but currently we have (at least) tests
    // that expect the pixelref to succeed even when there is a mismatch
    // with colortables. fix?
#if 0
    if (kIndex8_SkColorType == info.fColorType && nullptr == ctable) {
        return false;
    }
    if (kIndex8_SkColorType != info.fColorType && ctable) {
        return false;
    }
#endif
    return true;
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeDirect(const SkImageInfo& info,
                                               void* addr,
                                               size_t rowBytes,
                                               sk_sp<SkColorTable> ctable) {
    if (!is_valid(info, ctable.get())) {
        return nullptr;
    }
    return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes, std::move(ctable),
                                                  nullptr, nullptr));
}


 sk_sp<SkPixelRef> SkMallocPixelRef::MakeUsing(void*(*alloc)(size_t),
                                               const SkImageInfo& info,
                                               size_t requestedRowBytes,
                                               sk_sp<SkColorTable> ctable) {
    if (!is_valid(info, ctable.get())) {
        return nullptr;
    }

    // only want to permit 31bits of rowBytes
    int64_t minRB = (int64_t)info.minRowBytes64();
    if (minRB < 0 || !sk_64_isS32(minRB)) {
        return nullptr;    // allocation will be too large
    }
    if (requestedRowBytes > 0 && (int32_t)requestedRowBytes < minRB) {
        return nullptr;    // cannot meet requested rowbytes
    }

    int32_t rowBytes;
    if (requestedRowBytes) {
        rowBytes = SkToS32(requestedRowBytes);
    } else {
        rowBytes = minRB;
    }

    int64_t bigSize = (int64_t)info.height() * rowBytes;
    if (!sk_64_isS32(bigSize)) {
        return nullptr;
    }

    size_t size = sk_64_asS32(bigSize);
    SkASSERT(size >= info.getSafeSize(rowBytes));
    void* addr = alloc(size);
    if (nullptr == addr) {
        return nullptr;
    }

     return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes, std::move(ctable),
                                                   sk_free_releaseproc, nullptr));
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeAllocate(const SkImageInfo& info,
                                                size_t rowBytes,
                                                sk_sp<SkColorTable> ctable) {
    auto sk_malloc_nothrow = [](size_t size) { return sk_malloc_flags(size, 0); };
    return MakeUsing(sk_malloc_nothrow, info, rowBytes, std::move(ctable));
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeZeroed(const SkImageInfo& info,
                                               size_t rowBytes,
                                               sk_sp<SkColorTable> ctable) {
    return MakeUsing(sk_calloc, info, rowBytes, std::move(ctable));
}

static void sk_data_releaseproc(void*, void* dataPtr) {
    (static_cast<SkData*>(dataPtr))->unref();
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeWithProc(const SkImageInfo& info,
                                                 size_t rowBytes,
                                                 sk_sp<SkColorTable> ctable,
                                                 void* addr,
                                                 SkMallocPixelRef::ReleaseProc proc,
                                                 void* context) {
    if (!is_valid(info, ctable.get())) {
        if (proc) {
            proc(addr, context);
        }
        return nullptr;
    }
    return sk_sp<SkPixelRef>(new SkMallocPixelRef(info, addr, rowBytes, std::move(ctable),
                                                  proc, context));
}

sk_sp<SkPixelRef> SkMallocPixelRef::MakeWithData(const SkImageInfo& info,
                                                size_t rowBytes,
                                                sk_sp<SkColorTable> ctable,
                                                sk_sp<SkData> data) {
    SkASSERT(data != nullptr);
    if (!is_valid(info, ctable.get())) {
        return nullptr;
    }
    if ((rowBytes < info.minRowBytes()) || (data->size() < info.getSafeSize(rowBytes))) {
        return nullptr;
    }
    // must get this address before we call release
    void* pixels = const_cast<void*>(data->data());
    SkPixelRef* pr = new SkMallocPixelRef(info, pixels, rowBytes, std::move(ctable),
                                          sk_data_releaseproc, data.release());
    pr->setImmutable(); // since we were created with (immutable) data
    return sk_sp<SkPixelRef>(pr);
}

///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkColorTable> sanitize(const SkImageInfo& info, sk_sp<SkColorTable> ctable) {
    if (kIndex_8_SkColorType == info.colorType()) {
        SkASSERT(ctable);
    } else {
        ctable.reset(nullptr);
    }
    return ctable;
}

SkMallocPixelRef::SkMallocPixelRef(const SkImageInfo& info, void* storage,
                                   size_t rowBytes, sk_sp<SkColorTable> ctable,
                                   SkMallocPixelRef::ReleaseProc proc,
                                   void* context)
    : INHERITED(info, storage, rowBytes, sanitize(info, std::move(ctable)))
    , fReleaseProc(proc)
    , fReleaseProcContext(context)
{}


SkMallocPixelRef::~SkMallocPixelRef() {
    if (fReleaseProc != nullptr) {
        fReleaseProc(this->pixels(), fReleaseProcContext);
    }
}

#ifdef SK_SUPPORT_LEGACY_NO_ADDR_PIXELREF
bool SkMallocPixelRef::onNewLockPixels(LockRec* rec) {
    sk_throw(); // should never get here
    return true;
}

void SkMallocPixelRef::onUnlockPixels() {
    // nothing to do
}
#endif

size_t SkMallocPixelRef::getAllocatedSizeInBytes() const {
    return this->info().getSafeSize(this->rowBytes());
}

#ifdef SK_SUPPORT_LEGACY_PIXELREFFACTORY
SkMallocPixelRef* SkMallocPixelRef::NewWithData(const SkImageInfo& info,
                                                size_t rowBytes,
                                                SkColorTable* ctable,
                                                SkData* data) {
    return (SkMallocPixelRef*)MakeWithData(info, rowBytes, sk_ref_sp(ctable), sk_ref_sp(data)).release();
}
#endif
