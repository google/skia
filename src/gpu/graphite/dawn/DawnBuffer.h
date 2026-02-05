/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBuffer_DEFINED
#define skgpu_graphite_DawnBuffer_DEFINED

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "include/core/SkRefCnt.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

class DawnBuffer : public Buffer {
public:
    static sk_sp<DawnBuffer> Make(const DawnSharedContext*,
                                  size_t size,
                                  BufferType type,
                                  AccessPattern);

    bool isUnmappable() const override;

    const wgpu::Buffer& dawnBuffer() const { return fBuffer; }

private:
    DawnBuffer(const DawnSharedContext*, size_t size, wgpu::Buffer, void* mapAtCreationPtr);

    bool prepareForReturnToCache(Resource::TakeRefFunc takeRef, void* takeRefCtx) override;
    void onAsyncMap(GpuFinishedProc, GpuFinishedContext) override;
    void onMap() override;
    void onUnmap() override;

    template <typename StatusT, typename MessageT>
    void mapCallback(StatusT status, MessageT message);

    void freeGpuData() override;

    const DawnSharedContext* dawnSharedContext() const {
        return static_cast<const DawnSharedContext*>(this->sharedContext());
    }

    void setBackendLabel(char const* label) override;

    wgpu::Buffer fBuffer;
    skia_private::STArray<1, AutoCallback> fAsyncMapCallbacks;

    // Ensure that only one thread can call asyncMap().
    [[maybe_unused]] SingleOwner fSingleMapCaller;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnBuffer_DEFINED
