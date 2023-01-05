/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DBuffer.h"

#include "src/gpu/ganesh/d3d/GrD3DGpu.h"
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

static gr_cp<ID3D12Resource> make_d3d_buffer(GrD3DGpu* gpu,
                                             size_t size,
                                             GrGpuBufferType intendedType,
                                             GrAccessPattern accessPattern,
                                             D3D12_RESOURCE_STATES* resourceState,
                                             sk_sp<GrD3DAlloc>* alloc) {
    D3D12_HEAP_TYPE heapType;
    if (accessPattern == kStatic_GrAccessPattern) {
        SkASSERT(intendedType != GrGpuBufferType::kXferCpuToGpu &&
                 intendedType != GrGpuBufferType::kXferGpuToCpu);
        heapType = D3D12_HEAP_TYPE_DEFAULT;
        // Needs to be transitioned to appropriate state to be read in shader
        *resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
    } else {
        if (intendedType == GrGpuBufferType::kXferGpuToCpu) {
            heapType = D3D12_HEAP_TYPE_READBACK;
            // Cannot be changed
            *resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
        } else {
            heapType = D3D12_HEAP_TYPE_UPLOAD;
            // Cannot be changed
            // Includes VERTEX_AND_CONSTANT_BUFFER, INDEX_BUFFER, INDIRECT_ARGUMENT, and COPY_SOURCE
            *resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
    }

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;  // default alignment
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0; // Doesn't apply to buffers
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    gr_cp<ID3D12Resource> resource = gpu->memoryAllocator()->createResource(
            heapType, &bufferDesc, *resourceState, alloc, nullptr);

    return resource;
}

sk_sp<GrD3DBuffer> GrD3DBuffer::Make(GrD3DGpu* gpu, size_t size, GrGpuBufferType intendedType,
                                     GrAccessPattern accessPattern) {
    SkASSERT(!gpu->protectedContext() || (accessPattern != kStatic_GrAccessPattern));
    D3D12_RESOURCE_STATES resourceState;

    sk_sp<GrD3DAlloc> alloc;
    gr_cp<ID3D12Resource> resource = make_d3d_buffer(gpu, size, intendedType, accessPattern,
                                                     &resourceState, &alloc);
    if (!resource) {
        return nullptr;
    }

    return sk_sp<GrD3DBuffer>(new GrD3DBuffer(gpu, size, intendedType, accessPattern,
                                              std::move(resource), std::move(alloc),
                                              resourceState,
                                              /*label=*/"MakeD3DBuffer"));
}

GrD3DBuffer::GrD3DBuffer(GrD3DGpu* gpu, size_t size, GrGpuBufferType intendedType,
                         GrAccessPattern accessPattern, gr_cp<ID3D12Resource> bufferResource,
                         sk_sp<GrD3DAlloc> alloc,
                         D3D12_RESOURCE_STATES resourceState,
                         std::string_view label)
    : INHERITED(gpu, size, intendedType, accessPattern, label)
    , fResourceState(resourceState)
    , fD3DResource(std::move(bufferResource))
    , fAlloc(std::move(alloc)) {
    this->registerWithCache(skgpu::Budgeted::kYes);

    // TODO: persistently map UPLOAD resources?

    VALIDATE();
}

void GrD3DBuffer::setResourceState(const GrD3DGpu* gpu,
                                   D3D12_RESOURCE_STATES newResourceState) {
    if (newResourceState == fResourceState ||
        // GENERIC_READ encapsulates a lot of different read states
        (fResourceState == D3D12_RESOURCE_STATE_GENERIC_READ &&
         SkToBool(newResourceState | fResourceState))) {
        return;
    }

    D3D12_RESOURCE_TRANSITION_BARRIER barrier = {};
    barrier.pResource = this->d3dResource();
    barrier.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.StateBefore = fResourceState;
    barrier.StateAfter = newResourceState;

    gpu->addBufferResourceBarriers(this, 1, &barrier);

    fResourceState = newResourceState;
}

void GrD3DBuffer::releaseResource() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMapPtr) {
        this->unmap();
    }

    SkASSERT(fD3DResource);
    SkASSERT(fAlloc);
    fD3DResource.reset();
    fAlloc.reset();
}

void GrD3DBuffer::onRelease() {
    this->releaseResource();
    this->INHERITED::onRelease();
}

void GrD3DBuffer::onAbandon() {
    this->releaseResource();
    this->INHERITED::onAbandon();
}

void GrD3DBuffer::onMap(MapType type) {
    fMapPtr = this->internalMap(type, 0, this->size());
}

void GrD3DBuffer::onUnmap(MapType type) {
    this->internalUnmap(type, 0, this->size());
}

bool GrD3DBuffer::onClearToZero() {
    if (!fD3DResource) {
        return false;
    }

    if (this->accessPattern() == kStatic_GrAccessPattern) {
        GrStagingBufferManager::Slice slice =
                this->getD3DGpu()->stagingBufferManager()->allocateStagingBufferSlice(this->size());
        if (!slice.fBuffer) {
            return false;
        }
        std::memset(slice.fOffsetMapPtr, 0, this->size());
        this->setResourceState(this->getD3DGpu(), D3D12_RESOURCE_STATE_COPY_DEST);
        this->getD3DGpu()->currentCommandList()->copyBufferToBuffer(
                sk_ref_sp<GrD3DBuffer>(this),
                0,
                static_cast<const GrD3DBuffer*>(slice.fBuffer)->d3dResource(),
                slice.fOffset,
                this->size());
        return true;
    }

    void* ptr = this->internalMap(MapType::kWriteDiscard, 0, this->size());
    if (!ptr) {
        return false;
    }
    std::memset(ptr, 0, this->size());
    this->internalUnmap(MapType::kWriteDiscard, 0, this->size());

    return true;
}

bool GrD3DBuffer::onUpdateData(const void* src, size_t offset, size_t size, bool /*preserve*/) {
    if (!fD3DResource) {
        return false;
    }

    void* ptr = this->internalMap(MapType::kWriteDiscard, offset, size);
    if (!ptr) {
        return false;
    }
    if (this->accessPattern() == kStatic_GrAccessPattern) {
        // We should never call this method on static buffers in protected contexts.
        SkASSERT(!this->getD3DGpu()->protectedContext());
        //*** any alignment restrictions?
    }
    memcpy(ptr, src, size);
    this->internalUnmap(MapType::kWriteDiscard, offset, size);

    return true;
}

void* GrD3DBuffer::internalMap(MapType type, size_t offset, size_t size) {
    // TODO: if UPLOAD heap type, could be persistently mapped (i.e., this would be a no-op)
    SkASSERT(fD3DResource);
    SkASSERT(!this->isMapped());
    SkASSERT(offset + size <= this->size());

    VALIDATE();

    if (this->accessPattern() == kStatic_GrAccessPattern) {
        if (type == MapType::kRead) {
            return nullptr;
        }
        SkASSERT(!fStagingBuffer);
        GrStagingBufferManager::Slice slice =
                this->getD3DGpu()->stagingBufferManager()->allocateStagingBufferSlice(size);
        if (!slice.fBuffer) {
            return nullptr;
        }
        fStagingBuffer = static_cast<const GrD3DBuffer*>(slice.fBuffer)->d3dResource();
        fStagingOffset = slice.fOffset;
        VALIDATE();
        return slice.fOffsetMapPtr;
    }

    D3D12_RANGE range;
    range.Begin = offset;
    // The range passed here indicates the portion of the resource that may be
    // read. If we're only writing then pass an empty range.
    range.End = type == MapType::kRead ? offset + size : offset;
    void* result;
    fD3DResource->Map(0, &range, &result);
    if (result) {
        result = SkTAddOffset<void>(result, offset);
    }
    VALIDATE();
    return result;
}

void GrD3DBuffer::internalUnmap(MapType type, size_t offset, size_t size) {
    // TODO: if UPLOAD heap type, could be persistently mapped (i.e., this would be a no-op)
    SkASSERT(fD3DResource);
    SkASSERT(offset + size <= this->size());
    VALIDATE();

    if (this->accessPattern() == kStatic_GrAccessPattern) {
        SkASSERT(type != GrGpuBuffer::MapType::kRead);
        SkASSERT(fStagingBuffer);
        this->setResourceState(this->getD3DGpu(), D3D12_RESOURCE_STATE_COPY_DEST);
        this->getD3DGpu()->currentCommandList()->copyBufferToBuffer(
                sk_ref_sp<GrD3DBuffer>(this),
                offset,
                fStagingBuffer,
                fStagingOffset,
                size);
        fStagingBuffer = nullptr;
    } else {
        D3D12_RANGE range;
        range.Begin = offset;
        range.End = type == MapType::kWriteDiscard ? offset + size : offset;
        // For READBACK heaps, unmap requires an empty range
        SkASSERT(fResourceState != D3D12_RESOURCE_STATE_COPY_DEST || range.Begin == range.End);
        fD3DResource->Unmap(0, &range);
    }

    VALIDATE();
}

void GrD3DBuffer::onSetLabel() {
    SkASSERT(fD3DResource);
    if (!this->getLabel().empty()) {
        const std::wstring label = L"_Skia_" + GrD3DMultiByteToWide(this->getLabel());
        this->d3dResource()->SetName(label.c_str());
    }
}

#ifdef SK_DEBUG
void GrD3DBuffer::validate() const {
    SkASSERT(this->intendedType() == GrGpuBufferType::kVertex ||
             this->intendedType() == GrGpuBufferType::kIndex ||
             this->intendedType() == GrGpuBufferType::kDrawIndirect ||
             this->intendedType() == GrGpuBufferType::kXferCpuToGpu ||
             this->intendedType() == GrGpuBufferType::kXferGpuToCpu);
}
#endif
