/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlMemoryAllocatorImpl_DEFINED
#define GrMtlMemoryAllocatorImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/mtl/GrMtlMemoryAllocator.h"

#import <Metal/Metal.h>

class GrMtlMemoryAllocatorImpl : public GrMtlMemoryAllocator {
public:
    static sk_sp<GrMtlMemoryAllocator> Make(id<MTLDevice>);

    ~GrMtlMemoryAllocatorImpl() override {}

    id<MTLBuffer> createBuffer(NSUInteger length, MTLResourceOptions options,
                               sk_sp<GrMtlAlloc>* allocation) override;
    id<MTLTexture> createTexture(MTLTextureDescriptor* texDesc,
                                 sk_sp<GrMtlAlloc>* allocation) override;

    class Alloc : public GrMtlAlloc {
    public:
        Alloc() {}
        ~Alloc() override {
            // free allocaiton
        }
    private:
        friend class GrMtlMemoryAllocatorImpl;
        // allocation data goes here
    };

private:
    GrMtlMemoryAllocatorImpl(id<MTLDevice> device) : fDevice(device) {}

    __weak id<MTLDevice> fDevice;
};

#endif
