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


std::unique_ptr<GrD3DDescriptorTable>
        GrD3DDescriptorTableManager::createShaderOrConstantResourceTable(GrD3DGpu* gpu,
                                                                         unsigned int size) {
    return fCBVSRVDescriptorPool.allocateTable(gpu, size);
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DDescriptorTableManager::createSamplerTable(
        GrD3DGpu* gpu, unsigned int size) {
    return fSamplerDescriptorPool.allocateTable(gpu, size);
}

void GrD3DDescriptorTableManager::prepForSubmit(GrD3DCommandList* commandList) {
    fCBVSRVDescriptorPool.prepForSubmit(commandList);
    fSamplerDescriptorPool.prepForSubmit(commandList);
}

void GrD3DDescriptorTableManager::recycle(Heap* heap) {
    // wrap the heap in an sk_sp and take ownership of it
    sk_sp<Heap> wrappedHeap(heap);

    SkASSERT(heap);
    switch (heap->type()) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            fCBVSRVDescriptorPool.recycle(std::move(wrappedHeap));
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            fSamplerDescriptorPool.recycle(std::move(wrappedHeap));
            break;
        default:
            SkUNREACHABLE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrD3DDescriptorTableManager::Heap> GrD3DDescriptorTableManager::Heap::Make(
        GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors) {
    std::unique_ptr<GrD3DDescriptorHeap> heap =
            GrD3DDescriptorHeap::Make(gpu, type, numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    if (!heap) {
        return nullptr;
    }

    return sk_sp< GrD3DDescriptorTableManager::Heap>(new Heap(gpu, heap, type, numDescriptors));
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DDescriptorTableManager::Heap::allocateTable(
        unsigned int count) {
    SkASSERT(fNumDescriptors - fNextAvailable >= count);
    unsigned int startIndex = fNextAvailable;
    fNextAvailable += count;
    return std::unique_ptr<GrD3DDescriptorTable>(
            new GrD3DDescriptorTable(fHeap->getGPUHandle(startIndex),
                                     fHeap->handleIncrementSize(), count));
}

void GrD3DDescriptorTableManager::Heap::onRecycle() const {
    fGpu->resourceProvider().descriptorTableMgr()->recycle(const_cast<Heap*>(this));
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DDescriptorTableManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fHeapType(heapType) {
    sk_sp<Heap> heap = Heap::Make(gpu, fHeapType, kHeapSize);
    fDescriptorHeaps.push_back(heap);
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DDescriptorTableManager::HeapPool::allocateTable(
        GrD3DGpu* gpu, unsigned int count) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->canAllocate(count)) {
            return fDescriptorHeaps[i]->allocateTable(count);
        }
    }

    // need to allocate more space
    sk_sp<GrD3DDescriptorTableManager::Heap> heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, kHeapSize);
    fDescriptorHeaps.push_back(heap);

    return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateTable(count);
}

void GrD3DDescriptorTableManager::HeapPool::prepForSubmit(GrD3DCommandList* commandList) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        commandList->addRecycledResource(fDescriptorHeaps[i]);
    }
    fDescriptorHeaps.clear();
}

void GrD3DDescriptorTableManager::HeapPool::recycle(sk_sp<Heap> heap) {
    SkASSERT(heap);
    heap->reset();
    fDescriptorHeaps.push_back(heap);
}
