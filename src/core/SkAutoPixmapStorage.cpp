/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkAutoPixmapStorage.h"

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/private/SkAssert.h"
#include "include/private/SkMalloc.h"

#include <utility>

SkAutoPixmapStorage::SkAutoPixmapStorage() = default;

SkAutoPixmapStorage::~SkAutoPixmapStorage() = default;

SkAutoPixmapStorage::SkAutoPixmapStorage(SkAutoPixmapStorage&& other) { *this = std::move(other); }

SkAutoPixmapStorage& SkAutoPixmapStorage::operator=(SkAutoPixmapStorage&& other) {
    if (this != &other) {
        this->fStorage = std::move(other.fStorage);
        this->INHERITED::reset(other.info(), other.addr(), other.rowBytes());
        other.INHERITED::reset();
    }
    return *this;
}

size_t SkAutoPixmapStorage::AllocSize(const SkImageInfo& info, size_t* rowBytes) {
    size_t rb = info.minRowBytes();
    if (rowBytes) {
        *rowBytes = rb;
    }
    return info.computeByteSize(rb);
}

bool SkAutoPixmapStorage::tryAlloc(const SkImageInfo& info) {
    this->reset();

    size_t rb;
    size_t size = AllocSize(info, &rb);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return false;
    }
    void* pixels = sk_malloc_canfail(size);
    if (nullptr == pixels) {
        return false;
    }
    this->INHERITED::reset(info, pixels, rb);
    fStorage.reset(pixels);
    return true;
}

void SkAutoPixmapStorage::alloc(const SkImageInfo& info) {
    SkASSERT_RELEASE(this->tryAlloc(info));
}

void* SkAutoPixmapStorage::detachPixels() {
    if (!fStorage) {
        return nullptr;
    }

    void* data = fStorage.release();
    this->INHERITED::reset();

    return data;
}

sk_sp<SkData> SkAutoPixmapStorage::detachPixelsAsData() {
    if (!fStorage) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeFromMalloc(fStorage.release(), this->computeByteSize());
    this->INHERITED::reset();

    return data;
}
