/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkSpan.h"

#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStreamPriv.h"

#include <cstddef>
#include <cstring>
#include <new>

SkData::SkData(SkSpan<std::byte> span, ReleaseProc proc, void* context)
    : fReleaseProc(proc)
    , fReleaseProcContext(context)
    , fSpan(span)
{}

/** This constructor means we are inline with our fPtr's contents.
 *  Thus we set fPtr to point right after this.
 */
SkData::SkData(size_t size)
    : fReleaseProc(nullptr)
    , fReleaseProcContext(nullptr)
    , fSpan{(std::byte*)(this + 1), size}
{}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fSpan.data(), fReleaseProcContext);
    }
}

bool SkData::operator==(const SkData& other) const {
    if (this == &other) {
        return true;
    }
    return size() == other.size() && !sk_careful_memcmp(data(), other.data(), size());
}

size_t SkData::copyRange(size_t offset, size_t length, void* buffer) const {
    size_t available = this->size();
    if (offset >= available || 0 == length) {
        return 0;
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    if (buffer) {
        memcpy(buffer, this->bytes() + offset, length);
    }
    return length;
}


#define RETURN_EMPTY_FOR_ZERO_SIZE(size)    \
    do {                                    \
        if ((size) == 0) {                  \
            return SkData::MakeEmpty();     \
        }                                   \
    } while (false)

#define VALIDATE_SUBSET(size, offset, length)           \
    do {                                                \
        if (offset > size || length > size - offset) {  \
            return nullptr;                             \
        }                                               \
    } while (0)

sk_sp<SkData> SkData::shareSubset(size_t offset, size_t length) {
    VALIDATE_SUBSET(this->size(), offset, length);

    if (offset == 0 && length == this->size()) {
        return sk_ref_sp(this);
    }

    RETURN_EMPTY_FOR_ZERO_SIZE(length);

    this->ref();
    return SkData::MakeWithProc(this->bytes() + offset, length, [](const void*, void* ctx) {
        ((SkData*)ctx)->unref();
    }, this);
}

sk_sp<SkData> SkData::copySubset(size_t offset, size_t length) const {
    VALIDATE_SUBSET(this->size(), offset, length);

    return SkData::MakeWithCopy(this->bytes() + offset, length);
}

void SkData::operator delete(void* p) {
    ::operator delete(p);
}

sk_sp<SkData> SkData::PrivateNewWithCopy(const void* srcOrNull, size_t length) {
    if (0 == length) {
        return SkData::MakeEmpty();
    }

    const size_t actualLength = length + sizeof(SkData);
    SkASSERT_RELEASE(length < actualLength);  // Check for overflow.

    void* storage = ::operator new (actualLength);
    sk_sp<SkData> data(new (storage) SkData(length));
    if (srcOrNull) {
        memcpy(data->writable_data(), srcOrNull, length);
    }
    return data;
}

void SkData::NoopReleaseProc(const void*, void*) {}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> SkData::MakeEmpty() {
    static SkOnce once;
    static SkData* empty;

    once([]{ empty = new SkData({}, nullptr, nullptr); });
    return sk_ref_sp(empty);
}

// assumes fPtr was allocated via sk_malloc
static void sk_free_releaseproc(const void* ptr, void*) {
    sk_free(const_cast<void*>(ptr));
}

sk_sp<SkData> SkData::MakeFromMalloc(const void* data, size_t length) {
    std::byte* ptr = static_cast<std::byte*>(const_cast<void*>(data));
    return sk_sp<SkData>(new SkData({ptr, length}, sk_free_releaseproc, nullptr));
}

sk_sp<SkData> SkData::MakeWithCopy(const void* src, size_t length) {
    SkASSERT(src);
    return PrivateNewWithCopy(src, length);
}

sk_sp<SkData> SkData::MakeUninitialized(size_t length) {
    return PrivateNewWithCopy(nullptr, length);
}

sk_sp<SkData> SkData::MakeZeroInitialized(size_t length) {
    auto data = MakeUninitialized(length);
    if (length != 0) {
        memset(data->writable_data(), 0, data->size());
    }
    return data;
}

sk_sp<SkData> SkData::MakeWithProc(const void* data, size_t length, ReleaseProc proc, void* ctx) {
    std::byte* ptr = static_cast<std::byte*>(const_cast<void*>(data));
    return sk_sp<SkData>(new SkData({ptr, length}, proc, ctx));
}

// assumes fPtr was allocated with sk_fmmap
static void sk_mmap_releaseproc(const void* addr, void* ctx) {
    size_t length = reinterpret_cast<size_t>(ctx);
    sk_fmunmap(addr, length);
}

sk_sp<SkData> SkData::MakeFromFILE(FILE* f) {
    size_t size;
    void* addr = sk_fmmap(f, &size);
    if (nullptr == addr) {
        return nullptr;
    }

    return SkData::MakeWithProc(addr, size, sk_mmap_releaseproc, reinterpret_cast<void*>(size));
}

sk_sp<SkData> SkData::MakeFromFileName(const char path[]) {
    FILE* f = path ? sk_fopen(path, kRead_SkFILE_Flag) : nullptr;
    if (nullptr == f) {
        return nullptr;
    }
    auto data = MakeFromFILE(f);
    sk_fclose(f);
    return data;
}

sk_sp<SkData> SkData::MakeFromFD(int fd) {
    size_t size;
    void* addr = sk_fdmmap(fd, &size);
    if (nullptr == addr) {
        return nullptr;
    }
    return SkData::MakeWithProc(addr, size, sk_mmap_releaseproc, reinterpret_cast<void*>(size));
}

sk_sp<SkData> SkData::MakeWithCString(const char cstr[]) {
    size_t size;
    if (nullptr == cstr) {
        cstr = "";
        size = 1;
    } else {
        size = strlen(cstr) + 1;
    }
    return MakeWithCopy(cstr, size);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> SkData::MakeFromStream(SkStream* stream, size_t size) {
    // reduce the chance of OOM by checking that the stream has enough bytes to read from before
    // allocating that potentially large buffer.
    if (StreamRemainingLengthIsBelow(stream, size)) {
        return nullptr;
    }
    sk_sp<SkData> data(SkData::MakeUninitialized(size));
    if (stream->read(data->writable_data(), size) != size) {
        return nullptr;
    }
    return data;
}
