/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DDescriptorTableManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DDescriptorTableManager::GrD3DDescriptorTableManager(GrD3DGpu* gpu)
    : fShaderViewDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    , fSamplerDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

sk_sp<GrD3DDescriptorTable>
        GrD3DDescriptorTableManager::createShaderViewTable(GrD3DGpu* gpu, unsigned int size) {
    sk_sp<GrD3DDescriptorTable> table = fShaderViewDescriptorPool.allocateTable(gpu, size);
    return table;
}

sk_sp<GrD3DDescriptorTable> GrD3DDescriptorTableManager::createSamplerTable(
        GrD3DGpu* gpu, unsigned int size) {
    sk_sp<GrD3DDescriptorTable> table = fSamplerDescriptorPool.allocateTable(gpu, size);
    return table;
}

void GrD3DDescriptorTableManager::prepForSubmit(GrD3DGpu* gpu) {
    fShaderViewDescriptorPool.prepForSubmit(gpu);
    fSamplerDescriptorPool.prepForSubmit(gpu);
}

void GrD3DDescriptorTableManager::recycle(Heap* heap) {
    // wrap the heap in an sk_sp and take ownership of it
    sk_sp<Heap> wrappedHeap(heap);

    SkASSERT(heap);
    switch (heap->type()) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            fShaderViewDescriptorPool.recycle(std::move(wrappedHeap));
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
        GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int descriptorCount) {
    std::unique_ptr<GrD3DDescriptorHeap> heap =
            GrD3DDescriptorHeap::Make(gpu, type, descriptorCount,
                                      D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    if (!heap) {
        return nullptr;
    }

    return sk_sp< GrD3DDescriptorTableManager::Heap>(new Heap(gpu, heap, type, descriptorCount));
}

sk_sp<GrD3DDescriptorTable> GrD3DDescriptorTableManager::Heap::allocateTable(
        unsigned int count) {
    SkASSERT(fDescriptorCount - fNextAvailable >= count);
    unsigned int startIndex = fNextAvailable;
    fNextAvailable += count;
    return sk_sp<GrD3DDescriptorTable>(
            new GrD3DDescriptorTable(fHeap->getCPUHandle(startIndex).fHandle,
                                     fHeap->getGPUHandle(startIndex).fHandle,
                                     fHeap->descriptorHeap(), fType));
}

void GrD3DDescriptorTableManager::Heap::onRecycle() const {
    fGpu->resourceProvider().descriptorTableMgr()->recycle(const_cast<Heap*>(this));
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DDescriptorTableManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fHeapType(heapType)
    , fCurrentHeapDescriptorCount(kInitialHeapDescriptorCount) {
    sk_sp<Heap> heap = Heap::Make(gpu, fHeapType, fCurrentHeapDescriptorCount);
    fDescriptorHeaps.push_back(heap);
}

sk_sp<GrD3DDescriptorTable> GrD3DDescriptorTableManager::HeapPool::allocateTable(
        GrD3DGpu* gpu, unsigned int count) {
    // In back-to-front order, iterate through heaps until we find one we can allocate from.
    // Any heap we can't allocate from gets removed from the list.
    // If it was already used, it will have been added to the commandlist,
    // and then later recycled back to us.
    while (fDescriptorHeaps.size() > 0) {
        auto& heap = fDescriptorHeaps[fDescriptorHeaps.size() - 1];
        if (heap->canAllocate(count)) {
            if (!heap->used()) {
                gpu->currentCommandList()->addRecycledResource(heap);
            }
            return heap->allocateTable(count);
        }
        // No space in current heap, pop off list
        fDescriptorHeaps.pop_back();
    }

    // Out of available heaps, need to allocate a new one
    fCurrentHeapDescriptorCount = std::min(2*fCurrentHeapDescriptorCount, 2048u);
    sk_sp<GrD3DDescriptorTableManager::Heap> heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, fCurrentHeapDescriptorCount);
    gpu->currentCommandList()->addRecycledResource(heap);
    fDescriptorHeaps.push_back(heap);
    return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateTable(count);
}

sk_sp<GrD3DDescriptorTableManager::Heap>&
        GrD3DDescriptorTableManager::HeapPool::currentDescriptorHeap() {
    SkASSERT(fDescriptorHeaps.size() > 0);
    return fDescriptorHeaps[fDescriptorHeaps.size() - 1];
}

void GrD3DDescriptorTableManager::HeapPool::prepForSubmit(GrD3DGpu* gpu) {
    // Pop off the current descriptor heap
    if (fDescriptorHeaps[fDescriptorHeaps.size() - 1]->used()) {
        fDescriptorHeaps.pop_back();
    }

    if (fDescriptorHeaps.size() == 0) {
        fCurrentHeapDescriptorCount = std::min(fCurrentHeapDescriptorCount, 2048u);
        sk_sp<GrD3DDescriptorTableManager::Heap> heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, fCurrentHeapDescriptorCount);
        fDescriptorHeaps.push_back(heap);
    }
}

void GrD3DDescriptorTableManager::HeapPool::recycle(sk_sp<Heap> heap) {
    SkASSERT(heap);
    // only add heaps back if they match our current size
    // this purges any smaller heaps we no longer need
    if (heap->descriptorCount() == fCurrentHeapDescriptorCount) {
        heap->reset();
        fDescriptorHeaps.push_back(heap);
    }
}
