/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMallocPixelRef.h"

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
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
    return sk_make_sp<SkPixelRef>(info.width(), info.height(), addr, rowBytes);
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

    struct PixelRef final : public SkPixelRef {
        PixelRef(int w, int h, void* s, size_t r) : SkPixelRef(w, h, s, r) {}
        ~PixelRef() override { sk_free(this->pixels()); }
    };
    return sk_sp<SkPixelRef>(new PixelRef(info.width(), info.height(), addr, rowBytes));
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

    struct PixelRef final : public SkPixelRef {
        SkMallocPixelRef::ReleaseProc fReleaseProc;
        void* fReleaseProcContext;
        PixelRef(int w, int h, void* s, size_t r, SkMallocPixelRef::ReleaseProc proc, void* ctx)
            : SkPixelRef(w, h, s, r), fReleaseProc(proc), fReleaseProcContext(ctx) {}
        ~PixelRef() override {
            if (fReleaseProc) {
                fReleaseProc(this->pixels(), fReleaseProcContext);
            }
        }
    };
    return sk_sp<SkPixelRef>(new PixelRef(info.width(), info.height(), addr, rowBytes,
                                          proc, context));
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
    struct PixelRef final : public SkPixelRef {
        sk_sp<SkData> fData;
        PixelRef(int w, int h, void* s, size_t r, sk_sp<SkData> d)
            : SkPixelRef(w, h, s, r), fData(std::move(d)) {}
    };
    void* pixels = const_cast<void*>(data->data());
    sk_sp<SkPixelRef> pr(new PixelRef(info.width(), info.height(), pixels, rowBytes,
                                      std::move(data)));
    pr->setImmutable(); // since we were created with (immutable) data
    return pr;
}
