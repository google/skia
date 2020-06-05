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

void GrD3DDescriptorTableManager::bindHeaps(GrD3DDirectCommandList* commandList) {
    //if (fCBVSRVDescriptorPool.bindingNeedsUpdate() || fSamplerDescriptorPool.bindingNeedsUpdate()) {
        ID3D12DescriptorHeap* heaps[2] = {
            fCBVSRVDescriptorPool.currentHeap(),
            fSamplerDescriptorPool.currentHeap()
        };
        commandList->setDescriptorHeaps(2, heaps);
        fCBVSRVDescriptorPool.addResource(commandList);
        fSamplerDescriptorPool.addResource(commandList);
    //}
}

void GrD3DDescriptorTableManager::prepForSubmit() {
    fCBVSRVDescriptorPool.prepForSubmit();
    fSamplerDescriptorPool.prepForSubmit();
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
            GrD3DDescriptorHeap::Make(gpu, type, numDescriptors,
                                      D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
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
            new GrD3DDescriptorTable(fHeap->getCPUHandle(startIndex),
                                     fHeap->getGPUHandle(startIndex), count));
}

void GrD3DDescriptorTableManager::Heap::onRecycle() const {
    fGpu->resourceProvider().descriptorTableMgr()->recycle(const_cast<Heap*>(this));
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DDescriptorTableManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fHeapType(heapType)
    , fCurrentHeapSize(kInitialHeapSize)
    , fBindingNeedsUpdate(true) {
    sk_sp<Heap> heap = Heap::Make(gpu, fHeapType, fCurrentHeapSize);
    fDescriptorHeaps.push_back(heap);
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DDescriptorTableManager::HeapPool::allocateTable(
        GrD3DGpu* gpu, unsigned int count) {
    while (fDescriptorHeaps.size() > 0) {
        if (fDescriptorHeaps[fDescriptorHeaps.size() - 1]->canAllocate(count)) {
            return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateTable(count);
        }
        // no space in current heap, pop off list
        fDescriptorHeaps.pop_back();
        fBindingNeedsUpdate = true;
    }

    // out of heaps, need to allocate more space
    fCurrentHeapSize = SkTPin(2 * fCurrentHeapSize, 32, 2048);
    sk_sp<GrD3DDescriptorTableManager::Heap> heap =
            GrD3DDescriptorTableManager::Heap::Make(gpu, fHeapType, fCurrentHeapSize);
    fDescriptorHeaps.push_back(heap);
    return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateTable(count);
}

ID3D12DescriptorHeap* GrD3DDescriptorTableManager::HeapPool::currentHeap() const {
    if (fDescriptorHeaps.size() > 0) {
        return fDescriptorHeaps[fDescriptorHeaps.size() - 1]->d3dHeap();
    }
    return nullptr;
}

void GrD3DDescriptorTableManager::HeapPool::addResource(GrD3DCommandList* commandList) {
    SkASSERT(fDescriptorHeaps.size() > 0);
    commandList->addRecycledResource(fDescriptorHeaps[fDescriptorHeaps.size()-1]);
    fBindingNeedsUpdate = false;
}

void GrD3DDescriptorTableManager::HeapPool::recycle(sk_sp<Heap> heap) {
    SkASSERT(heap);
    if (heap->size() == fCurrentHeapSize) {
        heap->reset();
        fDescriptorHeaps.push_back(heap);
    }
}
