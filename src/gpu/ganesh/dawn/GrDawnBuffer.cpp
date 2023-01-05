/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnBuffer.h"

#include "src/gpu/ganesh/dawn/GrDawnAsyncWait.h"
#include "src/gpu/ganesh/dawn/GrDawnGpu.h"

namespace {
    wgpu::BufferUsage GrGpuBufferTypeToDawnUsageBit(GrGpuBufferType type) {
        switch (type) {
            case GrGpuBufferType::kVertex:
                return wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kIndex:
                return wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kXferCpuToGpu:
                return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            case GrGpuBufferType::kXferGpuToCpu:
                return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            default:
                SkASSERT(!"buffer type not supported by Dawn");
                return wgpu::BufferUsage::Vertex;
        }
    }
}

// static
sk_sp<GrDawnBuffer> GrDawnBuffer::Make(GrDawnGpu* gpu,
                                       size_t sizeInBytes,
                                       GrGpuBufferType type,
                                       GrAccessPattern pattern,
                                       std::string_view label) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeInBytes;
    bufferDesc.usage = GrGpuBufferTypeToDawnUsageBit(type);

    Mappable mappable = Mappable::kNot;
    if (bufferDesc.usage & wgpu::BufferUsage::MapRead) {
        SkASSERT(!SkToBool(bufferDesc.usage & wgpu::BufferUsage::MapWrite));
        mappable = Mappable::kReadOnly;
    } else if (bufferDesc.usage & wgpu::BufferUsage::MapWrite) {
        mappable = Mappable::kWriteOnly;
    }

    if (mappable == Mappable::kNot) {
        // onMap can still succeed by using a staging buffer that gets transferred to the real
        // buffer. updateData will use this same mechanism ("map", copy to staging buffer, "unmap").
        // The transfer must be 4 byte aligned. So ensure the real size of the buffer is 4 byte
        // aligned.
        bufferDesc.size = SkAlign4(bufferDesc.size);
        SkASSERT(gpu->caps()->transferFromBufferToBufferAlignment() == 4);
    }

    wgpu::Buffer buffer;
    void* mapPtr = nullptr;
    if (mappable == Mappable::kNot || mappable == Mappable::kReadOnly) {
        buffer = gpu->device().CreateBuffer(&bufferDesc);
    } else {
        bufferDesc.mappedAtCreation = true;
        buffer = gpu->device().CreateBuffer(&bufferDesc);
        mapPtr = buffer.GetMappedRange();
        if (!mapPtr) {
            SkDebugf("GrDawnBuffer: failed to map buffer at creation\n");
            return nullptr;
        }
    }

    return sk_sp<GrDawnBuffer>(new GrDawnBuffer(
            gpu, sizeInBytes, type, pattern, label, mappable, std::move(buffer), mapPtr));
}

GrDawnBuffer::GrDawnBuffer(GrDawnGpu* gpu,
                           size_t sizeInBytes,
                           GrGpuBufferType type,
                           GrAccessPattern pattern,
                           std::string_view label,
                           Mappable mappable,
                           wgpu::Buffer buffer,
                           void* mapPtr)
        : INHERITED(gpu, sizeInBytes, type, pattern, label)
        , fBuffer(std::move(buffer))
        , fMappable(mappable) {
    fMapPtr = mapPtr;

    // We want to make the blocking map in `onMap` available initially only for read-only buffers,
    // which are not mapped at creation or backed by a staging buffer which gets mapped
    // independently. Note that the blocking map procedure becomes available to both read-only and
    // write-only buffers once they get explicitly unmapped.
    fUnmapped = (mapPtr == nullptr && mappable == Mappable::kReadOnly);
    this->registerWithCache(skgpu::Budgeted::kYes);
}

void* GrDawnBuffer::internalMap(MapType type, size_t offset, size_t size) {
    if (fUnmapped) {
        SkASSERT(fMappable != Mappable::kNot);
        void* ptr = this->blockingMap(offset, size);
        if (!ptr) {
            SkDebugf("GrDawnBuffer: failed to map buffer\n");
            return nullptr;
        }
        fUnmapped = false;
        return SkTAddOffset<void>(ptr, offset);
    }

    if (fMappable == Mappable::kNot) {
        // Dawn requires that the offset and size be 4 byte aligned. If the offset is not
        // then we logically align the staging slice with the previous aligned value, adjust
        // the pointer into the slice that we return. We'll do the same adjustment when issuing the
        // transfer in internalUnmap so that the data winds up at the right offset.
        size_t r = offset & 0x3;
        size   += r;
        SkASSERT(type == MapType::kWriteDiscard);
        GrStagingBufferManager::Slice slice =
                this->getDawnGpu()->stagingBufferManager()->allocateStagingBufferSlice(
                        size, /*requiredAlignment=*/4);
        fStagingBuffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
        fStagingOffset = slice.fOffset;
        return SkTAddOffset<void>(slice.fOffsetMapPtr, r);
    }

    // We always create this buffers mapped or if they've been used on the gpu before we use the
    // async map callback to know when it is safe to reuse them. Thus by the time we get here
    // the buffer should always be mapped.
    SkASSERT(this->isMapped());
    return SkTAddOffset<void>(fMapPtr, offset);
}

void GrDawnBuffer::internalUnmap(MapType type, size_t offset, size_t size) {
    if (fMappable == Mappable::kNot) {
        SkASSERT(type == MapType::kWriteDiscard);
        // See comment in internalMap() about this adjustment.
        size_t r = offset & 0x3;
        offset -= r;
        size = SkAlign4(size + r);
        this->getDawnGpu()->getCopyEncoder().CopyBufferToBuffer(fStagingBuffer, fStagingOffset,
                                                                fBuffer, offset, size);
    } else {
        fBuffer.Unmap();
        fUnmapped = true;
    }
}

void GrDawnBuffer::onRelease() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMapPtr && fMappable != Mappable::kNot) {
        fBuffer.Unmap();
        fMapPtr = nullptr;
        fUnmapped = true;
    }

    this->GrGpuBuffer::onRelease();
}

bool GrDawnBuffer::onClearToZero() {
    void* ptr = this->internalMap(MapType::kWriteDiscard, 0, this->size());
    if (!ptr) {
        return false;
    }

    std::memset(ptr, 0, this->size());

    this->internalUnmap(MapType::kWriteDiscard, 0, this->size());

    return true;
}

void GrDawnBuffer::onMap(MapType type) {
    fMapPtr = this->internalMap(type, 0, this->size());
}

void GrDawnBuffer::onUnmap(MapType type) {
    this->internalUnmap(type, 0, this->size());
}

bool GrDawnBuffer::onUpdateData(const void* src, size_t offset, size_t size, bool /*preserve*/) {
    // Note that this subclass's impl of kWriteDiscard never actually discards.
    void* ptr = this->internalMap(MapType::kWriteDiscard, offset, size);
    if (!ptr) {
        return false;
    }

    memcpy(ptr, src, size);

    this->internalUnmap(MapType::kWriteDiscard, offset, size);

    return true;
}

GrDawnGpu* GrDawnBuffer::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnBuffer::mapAsync(MapAsyncCallback callback) {
    SkASSERT(fMappable != Mappable::kNot);
    SkASSERT(!fMapAsyncCallback);
    SkASSERT(!this->isMapped());

    fMapAsyncCallback = std::move(callback);
    fBuffer.MapAsync(
            (fMappable == Mappable::kReadOnly) ? wgpu::MapMode::Read : wgpu::MapMode::Write,
            0,
            wgpu::kWholeMapSize,
            [](WGPUBufferMapAsyncStatus status, void* userData) {
                static_cast<GrDawnBuffer*>(userData)->mapAsyncDone(status);
            },
            this);
}

void GrDawnBuffer::mapAsyncDone(WGPUBufferMapAsyncStatus status) {
    SkASSERT(fMapAsyncCallback);
    auto callback = std::move(fMapAsyncCallback);

    if (status != WGPUBufferMapAsyncStatus_Success) {
        SkDebugf("GrDawnBuffer: failed to map buffer (status: %u)\n", status);
        callback(false);
        return;
    }

    if (fMappable == Mappable::kReadOnly) {
        fMapPtr = const_cast<void*>(fBuffer.GetConstMappedRange());
    } else {
        fMapPtr = fBuffer.GetMappedRange();
    }

    if (this->isMapped()) {
        fUnmapped = false;
    }

    // Run the callback as the last step in this function since the callback can deallocate this
    // GrDawnBuffer.
    callback(this->isMapped());
}

void* GrDawnBuffer::blockingMap(size_t offset, size_t size) {
    SkASSERT(fMappable != Mappable::kNot);

    struct Context {
        GrDawnBuffer*    buffer;
        void*            result;
        GrDawnAsyncWait  wait;
    };

    Context context{this, nullptr, GrDawnAsyncWait{this->getDawnGpu()->device()}};

    // The offset must be a multiple of 8. If not back it up to the previous 8 byte multiple
    // and compensate by extending the size. In either case size must be a multiple of 4.
    SkASSERT(SkIsAlign4(offset));
    size_t r = offset & 0x7;
    offset -= r;
    size = SkAlign4(size + r);

    fBuffer.MapAsync(
            (fMappable == Mappable::kReadOnly) ? wgpu::MapMode::Read : wgpu::MapMode::Write,
            offset,
            size,
            [](WGPUBufferMapAsyncStatus status, void* userData) {
                auto* context = static_cast<Context*>(userData);
                if (status != WGPUBufferMapAsyncStatus_Success) {
                    context->result = nullptr;
                    context->wait.signal();
                    return;
                }
                auto* wgpuBuffer = &context->buffer->fBuffer;
                if (context->buffer->fMappable == Mappable::kReadOnly) {
                    context->result = const_cast<void*>(wgpuBuffer->GetConstMappedRange());
                } else {
                    context->result = wgpuBuffer->GetMappedRange();
                }
                if (context->result) {
                    context->buffer->fUnmapped = false;
                }
                context->wait.signal();
            },
            &context);

    context.wait.busyWait();

    return context.result ? SkTAddOffset<void>(context.result, r) : nullptr;
}
