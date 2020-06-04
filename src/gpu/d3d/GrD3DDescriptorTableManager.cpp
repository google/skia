/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DDescriptorTableManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DDescriptorTableManager::GrD3DDescriptorTableManager(GrD3DGpu* gpu)
    : fCBVSRVDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    , fSamplerDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}


GrD3DDescriptorTable GrD3DDescriptorTableManager::createShaderOrConstantResourceTable(
        GrD3DGpu* gpu, unsigned int size) {
    return fCBVSRVDescriptorPool.allocateTable(gpu, size);
}

GrD3DDescriptorTable GrD3DDescriptorTableManager::createSamplerTable(GrD3DGpu* gpu,
                                                                     unsigned int size) {
    return fSamplerDescriptorPool.allocateTable(gpu, size);
}

void GrD3DDescriptorTableManager::prepForSubmit(GrD3DCommandList* commandList) {

}

void GrD3DDescriptorTableManager::recycle(Heap* heap) {
    SkASSERT(heap);
    switch (heap->type()) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            fCBVSRVDescriptorPool.recycle(heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            fSamplerDescriptorPool.recycle(heap);
            break;
        default:
            SkUNREACHABLE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DDescriptorTableManager::Heap* GrD3DDescriptorTableManager::Heap::Make(
        GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors) {
    std::unique_ptr<GrD3DDescriptorHeap> heap =
            GrD3DDescriptorHeap::Make(gpu, type, numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    if (!heap) {
        return nullptr;
    }

    return new Heap(gpu, heap, type, numDescriptors);
}

GrD3DDescriptorTable GrD3DDescriptorTableManager::Heap::allocateTable(unsigned int count) {
    SkASSERT(fNumDescriptors - fNextAvailable >= count);
    GrD3DDescriptorTable table(fHeap->getGPUHandle(fNextAvailable), fHeap->handleIncrementSize(),
                               count);
    return table;
}

void GrD3DDescriptorTableManager::Heap::onRecycle() const {
//    fGpu->resourceProvider().recycleDescriptorTable(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DDescriptorTableManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fMaxAvailableDescriptors(32)
    , fHeapType(heapType) {
    GrD3DDescriptorTableManager::Heap* heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, fMaxAvailableDescriptors);
    fDescriptorHeaps.push_back(heap);
}

GrD3DDescriptorTable GrD3DDescriptorTableManager::HeapPool::allocateTable(GrD3DGpu* gpu,
                                                                          unsigned int count) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->canAllocate(count)) {
            return fDescriptorHeaps[i]->allocateTable(count);
        }
    }

    // need to allocate more space
    GrD3DDescriptorTableManager::Heap* heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, fMaxAvailableDescriptors);

    fDescriptorHeaps.push_back(heap);
    fMaxAvailableDescriptors *= 2;
    return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateTable(count);
}

void GrD3DDescriptorTableManager::HeapPool::reset() {
    fDescriptorHeaps.clear(); //*** might not be right
}

void GrD3DDescriptorTableManager::HeapPool::recycle(Heap* heap) {
    SkASSERT(heap);
    heap->reset();
    fDescriptorHeaps.push_back(heap);
}
