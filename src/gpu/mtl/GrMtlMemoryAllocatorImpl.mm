/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlMemoryAllocatorImpl.h"

sk_sp<GrMtlMemoryAllocator> GrMtlMemoryAllocatorImpl::Make(id<MTLDevice> device) {

    return sk_sp<GrMtlMemoryAllocator>(new GrMtlMemoryAllocatorImpl(device));
}

id<MTLBuffer> GrMtlMemoryAllocatorImpl::createBuffer(NSUInteger length, MTLResourceOptions options,
                                                     sk_sp<GrMtlAlloc>* allocation) {
    allocation->reset();
    return [fDevice newBufferWithLength:length options:options];
}

id<MTLTexture> GrMtlMemoryAllocatorImpl::createTexture(MTLTextureDescriptor* texDesc,
                                                       sk_sp<GrMtlAlloc>* allocation) {
    allocation->reset();
    return [fDevice newTextureWithDescriptor:texDesc];
}
