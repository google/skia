/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

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
};
#endif

sk_sp<Buffer> DawnBuffer::Make(const DawnSharedContext* sharedContext,
                               size_t size,
                               BufferType type,
                               PrioritizeGpuReads prioritizeGpuReads) {
    if (size <= 0) {
        return nullptr;
    }

    const DawnCaps* dawnCaps = sharedContext->dawnCaps();

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
        usage = wgpu::BufferUsage::Storage;
        break;
    }

    size = SkAlignTo(size, dawnCaps->getMinBufferAlignment());
    wgpu::BufferDescriptor desc;
#ifdef SK_DEBUG
    desc.label = kBufferTypeNames[static_cast<int>(type)];
#endif
    desc.usage = usage;
    desc.size  = size;
    // For wgpu::Buffer can be mapped at creation time for the initial data uploading. we should use
    // it for better performance?
    desc.mappedAtCreation = false;

    auto buffer = sharedContext->device().CreateBuffer(&desc);
    if (!buffer) {
        return {};
    }

    return sk_sp<Buffer>(new DawnBuffer(sharedContext,
                                        size,
                                        type,
                                        prioritizeGpuReads,
                                        std::move(buffer)));
}

DawnBuffer::DawnBuffer(const DawnSharedContext* sharedContext,
                       size_t size,
                       BufferType type,
                       PrioritizeGpuReads prioritizeGpuReads,
                       wgpu::Buffer buffer)
        : Buffer(sharedContext, size, type, prioritizeGpuReads)
        , fBuffer(std::move(buffer)) {}

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
    fMapPtr = fBuffer.GetMappedRange();
    SkASSERT(fMapPtr);
}

void DawnBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
    SkASSERT(fBuffer.GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite));

    fBuffer.Unmap();
    fMapPtr = nullptr;
}

void DawnBuffer::freeGpuData() {
    fBuffer = nullptr;
}

} // namespace skgpu::graphite
