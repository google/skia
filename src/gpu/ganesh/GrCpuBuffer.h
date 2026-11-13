/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCpuBuffer_DEFINED
#define GrCpuBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkMalloc.h"
#include "src/core/SkSafeMath.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrNonAtomicRef.h"

#include <cstddef>
#include <new>

class GrCpuBuffer final : public GrNonAtomicRef<GrCpuBuffer>, public GrBuffer {
public:
    static sk_sp<GrCpuBuffer> Make(size_t size) {
        if (!size) {
            return nullptr;
        }
        SkSafeMath sm;
        size_t combinedSize = sm.add(sizeof(GrCpuBuffer), size);
        if (!sm.ok()) {
            return nullptr;
        }
        void* mem = sk_malloc_canfail(combinedSize);
        if (!mem) {
            return nullptr;
        }
        return sk_sp<GrCpuBuffer>(
                new (mem) GrCpuBuffer(static_cast<char*>(mem) + sizeof(GrCpuBuffer), size));
    }

    static void operator delete(void* p) { sk_free(p); }

    void ref() const override { GrNonAtomicRef<GrCpuBuffer>::ref(); }
    void unref() const override { GrNonAtomicRef<GrCpuBuffer>::unref(); }
    size_t size() const override { return fSize; }
    bool isCpuBuffer() const override { return true; }

    char* data() { return reinterpret_cast<char*>(fData); }
    const char* data() const { return reinterpret_cast<const char*>(fData); }

private:
    GrCpuBuffer(void* data, size_t size) : fData(data), fSize(size) {}
    void* fData;
    size_t fSize;
};

#endif
