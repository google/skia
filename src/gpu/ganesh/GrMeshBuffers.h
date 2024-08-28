/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshBuffers_DEFINED
#define GrMeshBuffers_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkMeshPriv.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <cstddef>

template <typename Base, GrGpuBufferType> class GrMeshBuffer final : public Base {
public:
    GrMeshBuffer() = default;

    ~GrMeshBuffer() override;

    static sk_sp<Base> Make(GrDirectContext*, const void* data, size_t size);

    size_t size() const override {
        SkASSERT(fBuffer);
        return fBuffer->size();
    }

    bool isGaneshBacked() const override { return true; }

    sk_sp<const GrGpuBuffer> asGpuBuffer() const { return fBuffer; }

private:
    bool onUpdate(GrDirectContext*, const void* data, size_t offset, size_t size) override;

    sk_sp<GrGpuBuffer> fBuffer;
    GrDirectContext::DirectContextID fContextID;
};

namespace SkMeshPriv {
using GaneshIndexBuffer  = GrMeshBuffer<SkMeshPriv::IB, GrGpuBufferType::kIndex >;
using GaneshVertexBuffer = GrMeshBuffer<SkMeshPriv::VB, GrGpuBufferType::kVertex>;
}  // namespace SkMeshPriv

#endif
