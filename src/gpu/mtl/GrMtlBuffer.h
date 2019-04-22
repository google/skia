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

#import <metal/metal.h>

class GrMtlCaps;
class GrMtlGpu;

class GrMtlBuffer: public GrGpuBuffer {
public:
    static sk_sp<GrMtlBuffer> Make(GrMtlGpu*, size_t size, GrGpuBufferType intendedType,
                                   GrAccessPattern, const void* data = nullptr);

    ~GrMtlBuffer() override;

    id<MTLBuffer> mtlBuffer() const { return fMtlBuffer; }
    size_t offset() const { return fOffset; }

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
    size_t        fOffset;       // offset into shared buffer for dynamic buffers
    id<MTLBuffer> fMappedBuffer; // buffer used by static buffers for uploads

    typedef GrGpuBuffer INHERITED;
};

class GrMtlBufferManager {
public:
    GrMtlBufferManager(GrMtlGpu* gpu)
         : fGpu(gpu), fBufferAllocation(nil), fAllocationSize(0), fNextOffset(0) {}

    ~GrMtlBufferManager() {
        fBufferAllocation = nil; // Just to be sure
    }

    id<MTLBuffer> getDynamicAllocation(size_t size, size_t* offset);
    void setVertexBuffer(id<MTLRenderCommandEncoder>, const GrMtlBuffer*, size_t index);
    void setFragmentBuffer(id<MTLRenderCommandEncoder>, const GrMtlBuffer*, size_t index);
    void resetBindings();

private:
    GrMtlGpu*     fGpu;
    id<MTLBuffer> fBufferAllocation;
    size_t        fAllocationSize;
    size_t        fNextOffset;
    static constexpr size_t kNumBindings = GrMtlUniformHandler::kLastUniformBinding + 3;
    id<MTLBuffer> fBufferBindings[kNumBindings];
};

#endif
