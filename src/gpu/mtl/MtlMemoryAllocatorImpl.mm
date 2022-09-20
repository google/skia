/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/MtlMemoryAllocatorImpl.h"

namespace skgpu {

sk_sp<MtlMemoryAllocator> MtlMemoryAllocatorImpl::Make(id<MTLDevice> device) {
    return sk_sp<MtlMemoryAllocator>(new MtlMemoryAllocatorImpl(device));
}

id<MTLBuffer> MtlMemoryAllocatorImpl::newBufferWithLength(NSUInteger length,
                                                          MTLResourceOptions options,
                                                          sk_sp<MtlAlloc>* allocation) {
    // TODO: suballocate and fill in Alloc
    allocation->reset(new Alloc());
    return [fDevice newBufferWithLength:length options:options];
}

id<MTLTexture> MtlMemoryAllocatorImpl::newTextureWithDescriptor(MTLTextureDescriptor* texDesc,
                                                                sk_sp<MtlAlloc>* allocation) {
    // TODO: suballocate and fill in Alloc
    allocation->reset(new Alloc());
    return [fDevice newTextureWithDescriptor:texDesc];
}

} // namespace skgpu
