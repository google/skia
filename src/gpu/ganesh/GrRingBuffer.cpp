/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrRingBuffer.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAlign.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrResourceProvider.h"

#include <utility>

// Get offset into buffer that has enough space for size
// Returns fTotalSize if no space
size_t GrRingBuffer::getAllocationOffset(size_t size) {
    // capture current state locally (because fTail could be overwritten by the completion handler)
    size_t head, tail;
    head = fHead;
    tail = fTail;

    // The head and tail indices increment without bound, wrapping with overflow,
    // so we need to mod them down to the actual bounds of the allocation to determine
    // which blocks are available.
    size_t modHead = head & (fTotalSize - 1);
    size_t modTail = tail & (fTotalSize - 1);

    bool full = (head != tail && modHead == modTail);

    if (full) {
        return fTotalSize;
    }

    // case 1: free space lies at the beginning and/or the end of the buffer
    if (modHead >= modTail) {
        // check for room at the end
        if (fTotalSize - modHead < size) {
            // no room at the end, check the beginning
            if (modTail < size) {
                // no room at the beginning
                return fTotalSize;
            }
            // we are going to allocate from the beginning, adjust head to '0' position
            head += fTotalSize - modHead;
            modHead = 0;
        }
        // case 2: free space lies in the middle of the buffer, check for room there
    } else if (modTail - modHead < size) {
        // no room in the middle
        return fTotalSize;
    }

    fHead = SkAlignTo(head + size, fAlignment);
    return modHead;
}

GrRingBuffer::Slice GrRingBuffer::suballocate(size_t size) {
    fNewAllocation = true;
    if (fCurrentBuffer) {
        size_t offset = this->getAllocationOffset(size);
        if (offset < fTotalSize) {
            return { fCurrentBuffer.get(), offset };
        }

        // Try to grow allocation (old allocation will age out).
        fTotalSize *= 2;
        // Add current buffer to be tracked for next submit.
        fPreviousBuffers.push_back(std::move(fCurrentBuffer));
    }

    GrResourceProvider* resourceProvider = fGpu->getContext()->priv().resourceProvider();
    fCurrentBuffer = resourceProvider->createBuffer(fTotalSize,
                                                    fType,
                                                    kDynamic_GrAccessPattern,
                                                    GrResourceProvider::ZeroInit::kNo);

    SkASSERT(fCurrentBuffer);
    fHead = 0;
    fTail = 0;
    fGenID++;
    size_t offset = this->getAllocationOffset(size);
    SkASSERT(offset < fTotalSize);
    return { fCurrentBuffer.get(), offset };
}

// used when current command buffer/command list is submitted
void GrRingBuffer::startSubmit(GrGpu* gpu) {
    for (unsigned int i = 0; i < fPreviousBuffers.size(); ++i) {
        fPreviousBuffers[i]->unmap();
        gpu->takeOwnershipOfBuffer(std::move(fPreviousBuffers[i]));
    }
    fPreviousBuffers.clear();

    if (fNewAllocation) {
#ifdef SK_BUILD_FOR_MAC
        // Since we're using a Managed buffer on MacOS we need to unmap to write back to GPU
        // TODO: once we set up persistently mapped UPLOAD buffers on D3D, we can remove the
        // platform restriction.
        fCurrentBuffer->unmap();
#endif
        SubmitData* submitData = new SubmitData();
        submitData->fOwner = this;
        submitData->fLastHead = fHead;
        submitData->fGenID = fGenID;
        gpu->addFinishedCallback(skgpu::AutoCallback(FinishSubmit, submitData));
        fNewAllocation = false;
    }
}

// used when current command buffer/command list is completed
void GrRingBuffer::FinishSubmit(void* finishedContext) {
    GrRingBuffer::SubmitData* submitData = (GrRingBuffer::SubmitData*)finishedContext;
    if (submitData && submitData->fOwner && submitData->fGenID == submitData->fOwner->fGenID) {
        submitData->fOwner->fTail = submitData->fLastHead;
        submitData->fOwner = nullptr;
    }
    delete submitData;
}
