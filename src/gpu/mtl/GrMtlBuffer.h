/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlBuffer_DEFINED
#define GrMtlBuffer_DEFINED

#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"

#import <Metal/Metal.h>

class GrMtlCaps;
class GrMtlGpu;

class GrMtlBuffer: public GrGpuBuffer {
public:
    static sk_sp<GrMtlBuffer> Make(GrMtlGpu*, size_t size, GrGpuBufferType intendedType,
                                   GrAccessPattern, const void* data = nullptr);

    ~GrMtlBuffer() override;

    id<MTLBuffer> mtlBuffer() const { return fMtlBuffer; }

protected:
    GrMtlBuffer(GrMtlGpu*, size_t size, GrGpuBufferType intendedType, GrAccessPattern);

    void onAbandon() override;
    void onRelease() override;

private:
    GrMtlGpu* mtlGpu() const;

    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    void internalMap(size_t sizeInBytes);
    void internalUnmap(size_t sizeInBytes);

#ifdef SK_DEBUG
    void validate() const;
#endif

    bool fIsDynamic;
    id<MTLBuffer> fMtlBuffer;

    using INHERITED = GrGpuBuffer;
};

#endif
