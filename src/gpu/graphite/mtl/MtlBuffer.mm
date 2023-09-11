/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlBuffer.h"

#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

#ifdef SK_ENABLE_MTL_DEBUG_INFO
NSString* kBufferTypeNames[kBufferTypeCount] = {
        @"Vertex",
        @"Index",
        @"Xfer CPU to GPU",
        @"Xfer GPU to CPU",
        @"Uniform",
        @"Storage",
        @"Indirect",
        @"VertexStorage",
        @"IndexStorage",
};
#endif

sk_sp<Buffer> MtlBuffer::Make(const MtlSharedContext* sharedContext,
                              size_t size,
                              BufferType type,
                              AccessPattern accessPattern) {
    if (size <= 0) {
        return nullptr;
    }

    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        if (accessPattern == AccessPattern::kHostVisible) {
#ifdef SK_BUILD_FOR_MAC
            const MtlCaps& mtlCaps = sharedContext->mtlCaps();
            if (mtlCaps.isMac()) {
                options |= MTLResourceStorageModeManaged;
            } else {
                SkASSERT(mtlCaps.isApple());
                options |= MTLResourceStorageModeShared;
            }
#else
            options |= MTLResourceStorageModeShared;
#endif
        } else {
            options |= MTLResourceStorageModePrivate;
        }
    }

    sk_cfp<id<MTLBuffer>> buffer([sharedContext->device() newBufferWithLength:size
                                                                      options:options]);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    (*buffer).label = kBufferTypeNames[(int)type];
#endif

    return sk_sp<Buffer>(new MtlBuffer(sharedContext,
                                       size,
                                       std::move(buffer)));
}

MtlBuffer::MtlBuffer(const MtlSharedContext* sharedContext,
                     size_t size,
                     sk_cfp<id<MTLBuffer>> buffer)
        : Buffer(sharedContext, size)
        , fBuffer(std::move(buffer)) {}

void MtlBuffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());

    if ((*fBuffer).storageMode == MTLStorageModePrivate) {
        return;
    }

    fMapPtr = static_cast<char*>((*fBuffer).contents);
}

void MtlBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
#ifdef SK_BUILD_FOR_MAC
    if ((*fBuffer).storageMode == MTLStorageModeManaged) {
        [*fBuffer didModifyRange: NSMakeRange(0, this->size())];
    }
#endif
    fMapPtr = nullptr;
}

void MtlBuffer::freeGpuData() {
    fBuffer.reset();
}

} // namespace skgpu::graphite
