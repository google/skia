/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCpuBuffer_DEFINED
#define GrCpuBuffer_DEFINED

#include "GrBuffer.h"
#include "GrNonAtomicRef.h"

class GrCpuBuffer final : public GrNonAtomicRef<GrCpuBuffer>, public GrBuffer {
public:
    static sk_sp<GrCpuBuffer> Make(size_t size) {
        SkASSERT(size > 0);
        auto mem = ::operator new(sizeof(GrCpuBuffer) + size);
        return sk_sp<GrCpuBuffer>(new (mem) GrCpuBuffer((char*)mem + sizeof(GrCpuBuffer), size));
    }

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
