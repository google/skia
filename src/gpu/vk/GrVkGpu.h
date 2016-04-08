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
#include "vk/GrVkBackendContext.h"
#include "GrVkCaps.h"
#include "GrVkIndexBuffer.h"
#include "GrVkResourceProvider.h"
#include "GrVkVertexBuffer.h"
#include "GrVkUtil.h"

#include "shaderc/shaderc.h"
#include "vk/GrVkDefines.h"

class GrPipeline;
class GrNonInstancedMesh;

class GrVkBufferImpl;
class GrVkCommandBuffer;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkRenderPass;
class GrVkTexture;
struct GrVkInterface;

class GrVkGpu : public GrGpu {
public:
    static GrGpu* Create(GrBackendContext backendContext, const GrContextOptions& options,
                         GrContext* context);

    ~GrVkGpu() override;

    const GrVkInterface* vkInterface() const { return fBackendContext->fInterface; }
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

    void discard(GrRenderTarget*) override {}

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override;

    void onGetMultisampleSpecs(GrRenderTarget* rt,
                               const GrStencilSettings&,
                               int* effectiveSampleCnt,
                               SkAutoTDeleteArray<SkPoint>*);

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const override {
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

    void clearStencil(GrRenderTarget* target) override;

    void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) override {}

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
    GrVkGpu(GrContext* context, const GrContextOptions& options,
            const GrVkBackendContext* backendContext);

    void onResetContext(uint32_t resetBits) override {}

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                               const SkTArray<GrMipLevel>&) override;

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                                         const SkTArray<GrMipLevel>&) override { return NULL; }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership) override;

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                              GrWrapOwnership) override;
    GrRenderTarget* onWrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&) override { return NULL; }

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern) override;

    void onClear(GrRenderTarget*, const SkIRect& rect, GrColor color) override;

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) override;

    void onDraw(const GrPipeline&,
                const GrPrimitiveProcessor&,
                const GrMesh*,
                int meshCount) override;

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const SkTArray<GrMipLevel>&) override;

    bool onTransferPixels(GrSurface*,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override { return false; }

    void onResolveRenderTarget(GrRenderTarget* target) override {}

    sk_sp<GrVkPipelineState> prepareDrawState(const GrPipeline&,
                                              const GrPrimitiveProcessor&,
                                              GrPrimitiveType,
                                              const GrVkRenderPass&);

    // Bind vertex and index buffers
    void bindGeometry(const GrPrimitiveProcessor&, const GrNonInstancedMesh&);

    // Ends and submits the current command buffer to the queue and then creates a new command
    // buffer and begins it. If sync is set to kForce_SyncQueue, the function will wait for all
    // work in the queue to finish before returning.
    void submitCommandBuffer(SyncQueue sync);

    void copySurfaceAsCopyImage(GrSurface* dst,
                                GrSurface* src,
                                GrVkImage* dstImage,
                                GrVkImage* srcImage,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint);

    void copySurfaceAsBlit(GrSurface* dst,
                           GrSurface* src,
                           GrVkImage* dstImage,
                           GrVkImage* srcImage,
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

    SkAutoTUnref<const GrVkBackendContext> fBackendContext;
    SkAutoTUnref<GrVkCaps>                 fVkCaps;

    // These Vulkan objects are provided by the client, and also stored in fBackendContext.
    // They're copied here for convenient access.
    VkInstance                             fVkInstance;
    VkDevice                               fDevice;
    VkQueue                                fQueue;    // Must be Graphics queue

    // Created by GrVkGpu
    GrVkResourceProvider                   fResourceProvider;
    VkCommandPool                          fCmdPool;
    GrVkCommandBuffer*                     fCurrentCmdBuffer;
    VkPhysicalDeviceMemoryProperties       fPhysDevMemProps;

#ifdef ENABLE_VK_LAYERS
    // For reporting validation layer errors
    VkDebugReportCallbackEXT               fCallback;
#endif

    // Shaderc compiler used for compiling glsl in spirv. We only want to create the compiler once
    // since there is significant overhead to the first compile of any compiler.
    shaderc_compiler_t fCompiler;


    typedef GrGpu INHERITED;
};

#endif
