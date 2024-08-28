/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrMeshBuffers.h"

#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkMeshGanesh.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkMeshPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"

#include <cstring>
#include <utility>

template <typename Base, GrGpuBufferType Type> GrMeshBuffer<Base, Type>::~GrMeshBuffer() {
    GrResourceCache::ReturnResourceFromThread(std::move(fBuffer), fContextID);
}

template <typename Base, GrGpuBufferType Type>
sk_sp<Base> GrMeshBuffer<Base, Type>::Make(GrDirectContext* dc, const void* data, size_t size) {
    SkASSERT(dc);

    sk_sp<GrGpuBuffer> buffer = dc->priv().resourceProvider()->createBuffer(
            size,
            Type,
            kStatic_GrAccessPattern,
            data ? GrResourceProvider::ZeroInit::kNo : GrResourceProvider::ZeroInit::kYes);
    if (!buffer) {
        return nullptr;
    }

    if (data && !buffer->updateData(data, 0, size, /*preserve=*/false)) {
        return nullptr;
    }

    auto result = new GrMeshBuffer;
    result->fBuffer = std::move(buffer);
    result->fContextID = dc->directContextID();
    return sk_sp<Base>(result);
}

template <typename Base, GrGpuBufferType Type>
bool GrMeshBuffer<Base, Type>::onUpdate(GrDirectContext* dc,
                                        const void* data,
                                        size_t offset,
                                        size_t size) {
    if (!dc || dc != fBuffer->getContext()) {
        return false;
    }
    SkASSERT(!dc->abandoned());  // If dc is abandoned then fBuffer->getContext() should be null.

    if (!dc->priv().caps()->transferFromBufferToBufferSupport()) {
        auto ownedData = SkData::MakeWithCopy(data, size);
        dc->priv().drawingManager()->newBufferUpdateTask(
                std::move(ownedData), fBuffer, offset);
        return true;
    }

    sk_sp<GrGpuBuffer> tempBuffer;
    size_t tempOffset = 0;
    if (auto* sbm = dc->priv().getGpu()->stagingBufferManager()) {
        auto alignment = dc->priv().caps()->transferFromBufferToBufferAlignment();
        auto [sliceBuffer, sliceOffset, ptr] = sbm->allocateStagingBufferSlice(size, alignment);
        if (sliceBuffer) {
            std::memcpy(ptr, data, size);
            tempBuffer.reset(SkRef(sliceBuffer));
            tempOffset = sliceOffset;
        }
    }

    if (!tempBuffer) {
        tempBuffer = dc->priv().resourceProvider()->createBuffer(size,
                                                                 GrGpuBufferType::kXferCpuToGpu,
                                                                 kDynamic_GrAccessPattern,
                                                                 GrResourceProvider::ZeroInit::kNo);
        if (!tempBuffer) {
            return false;
        }
        if (!tempBuffer->updateData(data, 0, size, /*preserve=*/false)) {
            return false;
        }
    }

    dc->priv().drawingManager()->newBufferTransferTask(
            std::move(tempBuffer), tempOffset, fBuffer, offset, size);

    return true;
}

namespace SkMeshes {
sk_sp<SkMesh::IndexBuffer> MakeIndexBuffer(GrDirectContext* dc, const void* data, size_t size) {
    if (!dc) {
        // Fallback to a CPU buffer.
        return MakeIndexBuffer(data, size);
    }
    return SkMeshPriv::GaneshIndexBuffer::Make(dc, data, size);
}

sk_sp<SkMesh::IndexBuffer> CopyIndexBuffer(GrDirectContext* dc, sk_sp<SkMesh::IndexBuffer> src) {
    if (!src) {
        return nullptr;
    }
    auto* ib = static_cast<SkMeshPriv::IB*>(src.get());
    const void* data = ib->peek();
    if (!data) {
        return nullptr;
    }
    if (!dc) {
        return MakeIndexBuffer(data, ib->size());
    }
    return MakeIndexBuffer(dc, data, ib->size());
}

sk_sp<SkMesh::VertexBuffer> MakeVertexBuffer(GrDirectContext* dc, const void* data, size_t size) {
    if (!dc) {
        return MakeVertexBuffer(data, size);
    }
    return SkMeshPriv::GaneshVertexBuffer::Make(dc, data, size);
}

sk_sp<SkMesh::VertexBuffer> CopyVertexBuffer(GrDirectContext* dc, sk_sp<SkMesh::VertexBuffer> src) {
    if (!src) {
        return nullptr;
    }
    auto* vb = static_cast<SkMeshPriv::VB*>(src.get());
    const void* data = vb->peek();
    if (!data) {
        return nullptr;
    }
    if (!dc) {
        return MakeVertexBuffer(data, vb->size());
    }
    return MakeVertexBuffer(dc, data, vb->size());
}
}  // namespace SkMeshes
