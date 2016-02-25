/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkGpu_DEFINED
#define GrVkGpu_DEFINED

#include "GrGpu.h"
#include "GrGpuFactory.h"
#include "GrVkCaps.h"
#include "GrVkIndexBuffer.h"
#include "GrVkProgram.h"
#include "GrVkResourceProvider.h"
#include "GrVkVertexBuffer.h"
#include "GrVkUtil.h"

#include "shaderc/shaderc.h"
#include "vulkan/vulkan.h"

class GrPipeline;
class GrNonInstancedVertices;

class GrVkBufferImpl;
class GrVkCommandBuffer;
class GrVkPipeline;
class GrVkRenderPass;
class GrVkTexture;
struct GrVkInterface;

class GrVkGpu : public GrGpu {
public:
    // Currently passing in the inst so that we can properly delete it when we are done.
    // Normally this would be done by the client.
    GrVkGpu(GrContext* context, const GrContextOptions& options,
            VkPhysicalDevice physDev, VkDevice device, VkQueue queue, VkCommandPool cmdPool,
            VkInstance inst);
    ~GrVkGpu() override;

    const GrVkInterface* vkInterface() const { return fInterface.get(); }
    const GrVkCaps& vkCaps() const { return *fVkCaps; }

    VkDevice device() const { return fDevice; }
    VkQueue  queue() const { return fQueue; }
    VkCommandPool cmdPool() const { return fCmdPool; }
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties() const {
        return fPhysDevMemProps;
    }

    GrVkResourceProvider& resourceProvider() { return fResourceProvider;  }

    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override;

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override;

    void buildProgramDesc(GrProgramDesc*, const GrPrimitiveProcessor&,
                          const GrPipeline&) const override;

    void discard(GrRenderTarget*) override {
        SkDebugf("discard not yet implemented for Vulkan\n");
    }

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override;

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const override {
        SkDebugf("initCopySurfaceDstDesc not yet implemented for Vulkan\n");
        return false;
    }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config) override;
    bool isTestingOnlyBackendTexture(GrBackendObject id) const override;
    void deleteTestingOnlyBackendTexture(GrBackendObject id, bool abandonTexture) override;

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;

    void clearStencil(GrRenderTarget* target) override {
        SkDebugf("clearStencil not yet implemented for Vulkan\n");
    }

    void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) override {
        SkDebugf("drawDebugWireRect not yet implemented for Vulkan\n");
    }

    void addMemoryBarrier(VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask,
                          bool byRegion,
                          VkMemoryBarrier* barrier) const;
    void addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier) const;
    void addImageMemoryBarrier(VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion,
                               VkImageMemoryBarrier* barrier) const;
    
    shaderc_compiler_t shadercCompiler() const {
        return fCompiler;
    }

    void finishDrawTarget() override;

private:
    void onResetContext(uint32_t resetBits) override {
        SkDebugf("onResetContext not yet implemented for Vulkan\n");
    }

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                               const void* srcData, size_t rowBytes) override;

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                                         const void* srcData) override {
        SkDebugf("onCreateCompressedTexture not yet implemented for Vulkan\n");
        return NULL;
    }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership) override;

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                              GrWrapOwnership) override;
    GrRenderTarget* onWrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&,
                                                       GrWrapOwnership) override {
        SkDebugf("onWrapBackendTextureAsRenderTarget not yet implemented for Vulkan\n");
        return NULL;
    }

    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) override;
    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) override;
    GrTransferBuffer* onCreateTransferBuffer(size_t size, TransferType type) override;

    void onClear(GrRenderTarget*, const SkIRect& rect, GrColor color) override;

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) override {
        SkDebugf("onClearStencilClip not yet implemented for Vulkan\n");
    }

    void onDraw(const DrawArgs&, const GrNonInstancedVertices&) override;

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const void* buffer, size_t rowBytes) override;

    bool onTransferPixels(GrSurface*,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrTransferBuffer* buffer,
                          size_t offset, size_t rowBytes) override {
        SkDebugf("onTransferPixels not yet implemented for Vulkan\n");
        return false;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override {
        SkDebugf("onResolveRenderTarget not yet implemented for Vulkan\n");
    }

    // Bind vertex and index buffers
    void bindGeometry(const GrPrimitiveProcessor&, const GrNonInstancedVertices&);

    // Ends and submits the current command buffer to the queue and then creates a new command
    // buffer and begins it. If sync is set to kForce_SyncQueue, the function will wait for all 
    // work in the queue to finish before returning.
    void submitCommandBuffer(SyncQueue sync);

    void copySurfaceAsCopyImage(GrSurface* dst,
                                GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint);

    void copySurfaceAsDraw(GrSurface* dst,
                           GrSurface* src,
                           const SkIRect& srcRect,
                           const SkIPoint& dstPoint);

    // helper for onCreateTexture and writeTexturePixels
    bool uploadTexData(GrVkTexture* tex,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const void* data,
                       size_t rowBytes);

    SkAutoTUnref<const GrVkInterface> fInterface;
    SkAutoTUnref<GrVkCaps>            fVkCaps;
    VkPhysicalDeviceMemoryProperties  fPhysDevMemProps;
    VkDevice                          fDevice;
    VkQueue                           fQueue;   // for now, one queue
    VkCommandPool                     fCmdPool;
    GrVkCommandBuffer*                fCurrentCmdBuffer;
    GrVkResourceProvider              fResourceProvider;

    // Shaderc compiler used for compiling glsl in spirv. We only want to create the compiler once
    // since there is significant overhead to the first compile of any compiler.
    shaderc_compiler_t fCompiler;

    // This is only for our current testing and building. The client should be holding on to the
    // VkInstance.
    VkInstance                        fVkInstance;

    typedef GrGpu INHERITED;
};

#endif
