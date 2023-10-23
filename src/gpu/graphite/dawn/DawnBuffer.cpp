/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

#ifdef SK_DEBUG
static const char* kBufferTypeNames[kBufferTypeCount] = {
        "Vertex",
        "Index",
        "Xfer CPU to GPU",
        "Xfer GPU to CPU",
        "Uniform",
        "Storage",
        "Indirect",
        "VertexStorage",
        "IndexStorage",
};
#endif

sk_sp<DawnBuffer> DawnBuffer::Make(const DawnSharedContext* sharedContext,
                                   size_t size,
                                   BufferType type,
                                   AccessPattern accessPattern) {
    return DawnBuffer::Make(sharedContext,
                            size,
                            type,
                            accessPattern,
#ifdef SK_DEBUG
                            /*label=*/kBufferTypeNames[static_cast<int>(type)]
#else
                            /*label=*/nullptr
#endif
                            );
}

sk_sp<DawnBuffer> DawnBuffer::Make(const DawnSharedContext* sharedContext,
                                   size_t size,
                                   BufferType type,
                                   AccessPattern,
                                   const char* label) {
    if (size <= 0) {
        return nullptr;
    }

    wgpu::BufferUsage usage = wgpu::BufferUsage::None;
    switch (type) {
    case BufferType::kVertex:
        usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kIndex:
        usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kXferCpuToGpu:
        usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        break;
    case BufferType::kXferGpuToCpu:
        usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        break;
    case BufferType::kUniform:
        usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kStorage:
        usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst |
                wgpu::BufferUsage::CopySrc;
        break;
    case BufferType::kIndirect:
        usage = wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage;
        break;
    case BufferType::kVertexStorage:
        usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage;
        break;
    case BufferType::kIndexStorage:
        usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage;
        break;
    }

    wgpu::BufferDescriptor desc;
    desc.label = label;
    desc.usage = usage;
    desc.size  = size;
    // Specifying mappedAtCreation avoids clearing the buffer on the GPU which can cause MapAsync to
    // be very slow as it waits for GPU execution to complete.
    desc.mappedAtCreation = SkToBool(usage & wgpu::BufferUsage::MapWrite);

    auto buffer = sharedContext->device().CreateBuffer(&desc);
    if (!buffer) {
        return {};
    }

    return sk_sp<DawnBuffer>(new DawnBuffer(sharedContext, size, std::move(buffer)));
}

DawnBuffer::DawnBuffer(const DawnSharedContext* sharedContext, size_t size, wgpu::Buffer buffer)
        : Buffer(sharedContext, size), fBuffer(std::move(buffer)) {
    // Mapped at creation.
    if (fBuffer && fBuffer.GetMapState() == wgpu::BufferMapState::Mapped) {
        SkASSERT(fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite);
        fMapPtr = fBuffer.GetMappedRange();
        SkASSERT(fMapPtr);
    }
}

void DawnBuffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());
    SkASSERT((fBuffer.GetUsage() & wgpu::BufferUsage::MapRead) ||
             (fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite));

    DawnAsyncWait wait(dawnSharedContext()->device());
    bool isWrite = fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite;
    fBuffer.MapAsync(isWrite ? wgpu::MapMode::Write: wgpu::MapMode::Read,
                     0,
                     fBuffer.GetSize(),
                     [](WGPUBufferMapAsyncStatus s, void* userData) {
                        reinterpret_cast<DawnAsyncWait*>(userData)->signal();
                     },
                     &wait);
    wait.busyWait();
    if (isWrite) {
        fMapPtr = fBuffer.GetMappedRange();
    } else {
        // If buffer is only created with MapRead usage, Dawn only allows returning
        // constant pointer. We need to use const_cast as a workaround here.
        fMapPtr = const_cast<void*>(fBuffer.GetConstMappedRange());
    }
    SkASSERT(fMapPtr);
}

void DawnBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
    SkASSERT(fBuffer.GetMapState() == wgpu::BufferMapState::Mapped);

    fBuffer.Unmap();
    fMapPtr = nullptr;
}

void DawnBuffer::freeGpuData() {
    fBuffer = nullptr;
}

} // namespace skgpu::graphite
