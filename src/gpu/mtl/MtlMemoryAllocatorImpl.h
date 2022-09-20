/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlMemoryAllocatorImpl_DEFINED
#define skgpu_MtlMemoryAllocatorImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/mtl/MtlMemoryAllocator.h"

#import <Metal/Metal.h>

namespace skgpu {

class MtlMemoryAllocatorImpl : public MtlMemoryAllocator {
public:
    static sk_sp<MtlMemoryAllocator> Make(id<MTLDevice>);

    ~MtlMemoryAllocatorImpl() override {}

    id<MTLBuffer> newBufferWithLength(NSUInteger length, MTLResourceOptions options,
                                      sk_sp<skgpu::MtlAlloc>* allocation) override;
    id<MTLTexture> newTextureWithDescriptor(MTLTextureDescriptor* texDesc,
                                            sk_sp<skgpu::MtlAlloc>* allocation) override;

    class Alloc : public MtlAlloc {
    public:
        Alloc() {}
        ~Alloc() override {
            // TODO: free allocation
        }
    private:
        friend class MtlMemoryAllocatorImpl;
        // TODO: allocation data goes here
    };

private:
    MtlMemoryAllocatorImpl(id<MTLDevice> device) : fDevice(device) {}

    id<MTLDevice> fDevice;
};

} // namespace skgpu

#endif // skgpu_MtlMemoryAllocatorImpl_DEFINED
