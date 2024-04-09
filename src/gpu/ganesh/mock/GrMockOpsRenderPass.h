/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOpsRenderPass_DEFINED
#define GrMockOpsRenderPass_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

#include <array>
#include <cstddef>
#include <cstdint>

class GrBuffer;
class GrGeometryProcessor;
class GrGpu;
class GrOpFlushState;
class GrPipeline;
class GrProgramInfo;
class GrScissorState;
class GrSurfaceProxy;
enum GrSurfaceOrigin : int;
struct SkIRect;
struct SkRect;

class GrMockOpsRenderPass : public GrOpsRenderPass {
public:
    GrMockOpsRenderPass(GrMockGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                        LoadAndStoreInfo colorInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu)
            , fColorLoadOp(colorInfo.fLoadOp) {
    }

    GrGpu* gpu() override { return fGpu; }
    void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) override {}

    int numDraws() const { return fNumDraws; }

private:
    void onBegin() override {
        if (GrLoadOp::kClear == fColorLoadOp) {
            this->markRenderTargetDirty();
        }
    }
    bool onBindPipeline(const GrProgramInfo&, const SkRect&) override { return true; }
    void onSetScissorRect(const SkIRect&) override {}
    bool onBindTextures(const GrGeometryProcessor&,
                        const GrSurfaceProxy* const geomProcTextures[],
                        const GrPipeline&) override {
        return true;
    }
    void onBindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                       sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart) override {}
    void onDraw(int, int) override { this->noopDraw(); }
    void onDrawIndexed(int, int, uint16_t, uint16_t, int) override { this->noopDraw(); }
    void onDrawInstanced(int, int, int, int) override { this->noopDraw(); }
    void onDrawIndexedInstanced(int, int, int, int, int) override { this->noopDraw(); }
    void onDrawIndirect(const GrBuffer*, size_t, int) override { this->noopDraw(); }
    void onDrawIndexedIndirect(const GrBuffer*, size_t, int) override { this->noopDraw(); }
    void onClear(const GrScissorState& scissor, std::array<float, 4>) override {
        this->markRenderTargetDirty();
    }
    void onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) override {}
    void noopDraw() {
        this->markRenderTargetDirty();
        ++fNumDraws;
    }
    void markRenderTargetDirty() {
        if (auto* tex = fRenderTarget->asTexture()) {
            tex->markMipmapsDirty();
        }
    }

    GrMockGpu* fGpu;
    GrLoadOp fColorLoadOp;
    int fNumDraws = 0;

    using INHERITED = GrOpsRenderPass;
};

#endif
