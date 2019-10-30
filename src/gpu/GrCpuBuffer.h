/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCpuBuffer_DEFINED
#define GrCpuBuffer_DEFINED

#include "src/core/SkSafeMath.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrNonAtomicRef.h"

class GrCpuBuffer final : public GrNonAtomicRef<GrCpuBuffer>, public GrBuffer {
public:
    static sk_sp<GrCpuBuffer> Make(size_t size) {
        SkASSERT(size > 0);
        SkSafeMath sm;
        size_t combinedSize = sm.add(sizeof(GrCpuBuffer), size);
        if (!sm.ok()) {
            SK_ABORT("Buffer size is too big.");
        }
        auto mem = ::operator new(combinedSize);
        return sk_sp<GrCpuBuffer>(new (mem) GrCpuBuffer((char*)mem + sizeof(GrCpuBuffer), size));
    }

    // TODO(b/30449950): use sized delete once P0722R3 is available
    static void operator delete(void* p) { ::operator delete(p); }

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
