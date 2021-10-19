/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

namespace skgpu {
class Gpu;
class Texture;

struct AttachmentDesc {
    sk_sp<Texture> fTexture; // the ref on this will be taken by the command buffer
    LoadOp fLoadOp;
    StoreOp fStoreOp;
};

struct RenderPassDesc {
    AttachmentDesc fColorAttachment;
    std::array<float, 4> fClearColor;
    AttachmentDesc fColorResolveAttachment;

    AttachmentDesc fStencilDepthAttachment;
    uint32_t fClearStencil;
    float fClearDepth;

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
};

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {
        this->releaseResources();
    }

    bool hasWork() { return fHasWork; }

    virtual void beginRenderPass(const RenderPassDesc&) = 0;
    virtual void endRenderPass() = 0;

protected:
    CommandBuffer();

    void trackResource(sk_sp<SkRefCnt> resource) {
        fTrackedResources.push_back(std::move(resource));
    }
    void releaseResources();

    bool fHasWork = false;

private:
    inline static constexpr int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<SkRefCnt>> fTrackedResources;

};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
