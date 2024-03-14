/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/Log.h"
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
                                   AccessPattern accessPattern,
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
        usage = wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage |
                wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kVertexStorage:
        usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage;
        break;
    case BufferType::kIndexStorage:
        usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage;
        break;
    }

    if (sharedContext->caps()->drawBufferCanBeMapped() &&
        accessPattern == AccessPattern::kHostVisible &&
        type != BufferType::kXferGpuToCpu) {
        usage |= wgpu::BufferUsage::MapWrite;
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

    void* mappedAtCreationPtr = nullptr;
    if (desc.mappedAtCreation) {
        mappedAtCreationPtr = buffer.GetMappedRange();
        SkASSERT(mappedAtCreationPtr);
    }

    return sk_sp<DawnBuffer>(new DawnBuffer(sharedContext,
                                            size,
                                            std::move(buffer),
                                            mappedAtCreationPtr));
}

DawnBuffer::DawnBuffer(const DawnSharedContext* sharedContext,
                       size_t size,
                       wgpu::Buffer buffer,
                       void* mappedAtCreationPtr)
        : Buffer(sharedContext,
                 size,
                 /*commandBufferRefsAsUsageRefs=*/buffer.GetUsage() & wgpu::BufferUsage::MapWrite)
        , fBuffer(std::move(buffer)) {
    fMapPtr = mappedAtCreationPtr;
}

#if defined(__EMSCRIPTEN__)
void DawnBuffer::prepareForReturnToCache(const std::function<void()>& takeRef) {
    // This function is only useful for Emscripten where we have to pre-map the buffer
    // once it is returned to the cache.
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());

    // This implementation is almost Dawn-agnostic. However, Buffer base class doesn't have any
    // way of distinguishing a buffer that is mappable for writing from one mappable for reading.
    // We only need to re-map the former.
    if (!(fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite)) {
        return;
    }
    // We cannot start an async map while the GPU is still using the buffer. We asked that
    // our Resource convert command buffer refs to usage refs. So we should never have any
    // command buffer refs.
    SkASSERT(!this->debugHasCommandBufferRef());
    // Note that the map state cannot change on another thread when we are here. We got here
    // because there were no UsageRefs on the buffer but async mapping holds a UsageRef until it
    // completes.
    if (this->isMapped()) {
        return;
    }
    takeRef();
    this->asyncMap([](void* ctx, skgpu::CallbackResult result) {
                       sk_sp<DawnBuffer> buffer(static_cast<DawnBuffer*>(ctx));
                       if (result != skgpu::CallbackResult::kSuccess) {
                           buffer->setDeleteASAP();
                       }
                   },
                   this);
}

void DawnBuffer::onAsyncMap(GpuFinishedProc proc, GpuFinishedContext ctx) {
    // This function is only useful for Emscripten where we have to use asyncMap().
    SkASSERT(this->sharedContext()->caps()->bufferMapsAreAsync());

    if (proc) {
        SkAutoMutexExclusive ex(fAsyncMutex);
        if (this->isMapped()) {
            proc(ctx, CallbackResult::kSuccess);
            return;
        }
        fAsyncMapCallbacks.push_back(RefCntedCallback::Make(proc, ctx));
    }
    if (this->isUnmappable()) {
        return;
    }
    SkASSERT(fBuffer);
    SkASSERT((fBuffer.GetUsage() & wgpu::BufferUsage::MapRead) ||
             (fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite));
    SkASSERT(fBuffer.GetMapState() == wgpu::BufferMapState::Unmapped);
    bool isWrite = fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite;
    auto buffer = sk_ref_sp(this);

    fBuffer.MapAsync(
            isWrite ? wgpu::MapMode::Write : wgpu::MapMode::Read,
            0,
            fBuffer.GetSize(),
            [](WGPUBufferMapAsyncStatus s, void* userData) {
                sk_sp<DawnBuffer> buffer(static_cast<DawnBuffer*>(userData));
                buffer->mapCallback(s);
            },
            buffer.release());
}

#endif // defined(__EMSCRIPTEN__)

void DawnBuffer::onMap() {
#if defined(__EMSCRIPTEN__)
    SKGPU_LOG_W("Synchronous buffer mapping not supported in Dawn. Failing map request.");
#else
    SkASSERT(!this->sharedContext()->caps()->bufferMapsAreAsync());
    SkASSERT(fBuffer);
    SkASSERT((fBuffer.GetUsage() & wgpu::BufferUsage::MapRead) ||
             (fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite));
    bool isWrite = fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite;

    // Use wgpu::Future and wait with timeout=0 to trigger callback immediately.
    // The buffer must already be free of any GPU use at this point.
    wgpu::BufferMapCallbackInfo callbackInfo{};
    callbackInfo.mode = wgpu::CallbackMode::AllowSpontaneous;
    callbackInfo.userdata = this;
    callbackInfo.callback = [](WGPUBufferMapAsyncStatus s, void* userData) {
        auto buffer = static_cast<DawnBuffer*>(userData);
        buffer->mapCallback(s);
    };

    wgpu::FutureWaitInfo mapWaitInfo{};

    mapWaitInfo.future = fBuffer.MapAsync(isWrite ? wgpu::MapMode::Write : wgpu::MapMode::Read,
                                          0,
                                          fBuffer.GetSize(),
                                          callbackInfo);

    wgpu::Device device = static_cast<const DawnSharedContext*>(sharedContext())->device();
    [[maybe_unused]] auto status =
            device.GetAdapter().GetInstance().WaitAny(1, &mapWaitInfo, /*timeoutNS=*/0);

    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(mapWaitInfo.completed);
#endif  // defined(__EMSCRIPTEN__)
}

void DawnBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isUnmappable());

    fMapPtr = nullptr;
    fBuffer.Unmap();
}

void DawnBuffer::mapCallback(WGPUBufferMapAsyncStatus status) {
    SkAutoMutexExclusive em(this->fAsyncMutex);
    if (status == WGPUBufferMapAsyncStatus_Success) {
        if (this->fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite) {
            this->fMapPtr = this->fBuffer.GetMappedRange();
        } else {
            // If buffer is only created with MapRead usage, Dawn only allows returning
            // constant pointer. We need to use const_cast as a workaround here.
            this->fMapPtr = const_cast<void*>(this->fBuffer.GetConstMappedRange());
        }
    } else {
        const char* statusStr;
        Priority priority = Priority::kError;
        switch (status) {
            case WGPUBufferMapAsyncStatus_ValidationError:
                statusStr = "ValidationError";
                break;
            case WGPUBufferMapAsyncStatus_Unknown:
                statusStr = "Unknown";
                break;
            case WGPUBufferMapAsyncStatus_DeviceLost:
                statusStr = "DeviceLost";
                break;
            case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback:
                statusStr = "DestroyedBeforeCallback";
                priority = Priority::kDebug;
                break;
            case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback:
                statusStr = "UnmappedBeforeCallback";
                priority = Priority::kDebug;
                break;
            case WGPUBufferMapAsyncStatus_MappingAlreadyPending:
                statusStr = "MappingAlreadyPending";
                break;
            case WGPUBufferMapAsyncStatus_OffsetOutOfRange:
                statusStr = "OffsetOutOfRange";
                break;
            case WGPUBufferMapAsyncStatus_SizeOutOfRange:
                statusStr = "SizeOutOfRange";
                break;
            default:
                statusStr = "<Other>";
                break;
        }
        SKGPU_LOG(priority, "Buffer async map failed with status %s.", statusStr);
        for (auto& cb : this->fAsyncMapCallbacks) {
            cb->setFailureResult();
        }
    }
    this->fAsyncMapCallbacks.clear();
}

bool DawnBuffer::isUnmappable() const {
    return fBuffer.GetMapState() != wgpu::BufferMapState::Unmapped;
}

void DawnBuffer::freeGpuData() {
    fBuffer = nullptr;
}

} // namespace skgpu::graphite
