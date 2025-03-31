/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGpu_DEFINED
#define GrGLGpu_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorData.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuResource.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/GrWindowRectsState.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLContext.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLFinishCallbacks.h"
#include "src/gpu/ganesh/gl/GrGLRenderTarget.h"
#include "src/gpu/ganesh/gl/GrGLTexture.h"
#include "src/gpu/ganesh/gl/GrGLTypesPriv.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "src/gpu/ganesh/gl/GrGLVertexArray.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

class GrAttachment;
class GrBackendSemaphore;
class GrBuffer;
class GrDirectContext;
class GrGLBuffer;
class GrGLOpsRenderPass;
class GrGLProgram;
class GrGpuBuffer;
class GrProgramInfo;
class GrRenderTarget;
class GrSemaphore;
class GrStagingBufferManager;
class GrSurface;
class GrSurfaceProxy;
class GrTexture;
class SkData;
enum class SkTextureCompressionType;
struct GrContextOptions;
struct SkIPoint;
struct SkIRect;
struct SkISize;

namespace SkSL { enum class GLSLGeneration; }

namespace SkSurfaces { enum class BackendSurfaceAccess; }

namespace skgpu {
class AutoCallback;
class MutableTextureState;
class RefCntedCallback;
class Swizzle;
enum class Budgeted : bool;
enum class Mipmapped : bool;
}

class GrGLGpu final : public GrGpu {
public:
    static std::unique_ptr<GrGpu> Make(sk_sp<const GrGLInterface>,
                                       const GrContextOptions&,
                                       GrDirectContext*);
    ~GrGLGpu() override;

    void disconnect(DisconnectType) override;

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    const GrGLContext& glContext() const { return *fGLContext; }

    const GrGLInterface* glInterface() const { return fGLContext->glInterface(); }
    const GrGLContextInfo& ctxInfo() const { return *fGLContext; }
    GrGLStandard glStandard() const { return fGLContext->standard(); }
    GrGLVersion glVersion() const { return fGLContext->version(); }
    SkSL::GLSLGeneration glslGeneration() const { return fGLContext->glslGeneration(); }
    const GrGLCaps& glCaps() const { return *fGLContext->caps(); }

    GrStagingBufferManager* stagingBufferManager() override { return fStagingBufferManager.get(); }

    // Used by GrGLProgram to configure OpenGL state.
    void bindTexture(int unitIdx, GrSamplerState samplerState, const skgpu::Swizzle&, GrGLTexture*);

    // These functions should be used to bind GL objects. They track the GL state and skip redundant
    // bindings. Making the equivalent glBind calls directly will confuse the state tracking.
    void bindVertexArray(GrGLuint id) {
        fHWVertexArrayState.setVertexArrayID(this, id);
    }

    // These callbacks update state tracking when GL objects are deleted. They are called from
    // GrGLResource onRelease functions.
    void notifyVertexArrayDelete(GrGLuint id) {
        fHWVertexArrayState.notifyVertexArrayDelete(id);
    }

    // Binds a buffer to the GL target corresponding to 'type', updates internal state tracking, and
    // returns the GL target the buffer was bound to.
    // When 'type' is kIndex_GrBufferType, this function will also implicitly bind the default VAO.
    // If the caller wishes to bind an index buffer to a specific VAO, it can call glBind directly.
    GrGLenum bindBuffer(GrGpuBufferType type, const GrBuffer*);

    // Flushes state from GrProgramInfo to GL. Returns false if the state couldn't be set.
    bool flushGLState(GrRenderTarget*, bool useMultisampleFBO, const GrProgramInfo&);
    void flushScissorRect(const SkIRect& scissor, int rtHeight, GrSurfaceOrigin);

    // The flushRenderTarget methods will all set the initial viewport to the full extent of the
    // backing render target.
    void flushViewport(const SkIRect& viewport, int rtHeight, GrSurfaceOrigin);

    // Returns the last program bound by flushGLState(), or nullptr if a different program has since
    // been put into use via some other method (e.g., resetContext, copySurfaceAsDraw).
    // The returned GrGLProgram can be used for binding textures and vertex attributes.
    GrGLProgram* currentProgram() {
        this->handleDirtyContext();
        return fHWProgram.get();
    }

    // Binds the vertex array that should be used for internal draws, enables 'numAttribs' vertex
    // arrays, and flushes the desired primitive restart settings. If an index buffer is provided,
    // it will be bound to the vertex array. Otherwise the index buffer binding will be left
    // unchanged.
    //
    // NOTE: This binds the default VAO (ID=zero) unless we are on a core profile, in which case we
    // use a placeholder array instead.
    GrGLAttribArrayState* bindInternalVertexArray(const GrBuffer* indexBuffer, int numAttribs,
                                                  GrPrimitiveRestart primitiveRestart) {
        auto* attribState = fHWVertexArrayState.bindInternalVertexArray(this, indexBuffer);
        attribState->enableVertexArrays(this, numAttribs, primitiveRestart);
        return attribState;
    }

    // Applies any necessary workarounds and returns the GL primitive type to use in draw calls.
    GrGLenum prepareToDraw(GrPrimitiveType primitiveType);

    using ResolveDirection = GrGLRenderTarget::ResolveDirection;

    // Resolves the render target's single sample FBO into the MSAA, or vice versa.
    // If glCaps.framebufferResolvesMustBeFullSize() is true, resolveRect must be equal the render
    // target's bounds rect.
    // If blitting single to MSAA, glCaps.canResolveSingleToMSAA() must be true.
    void resolveRenderFBOs(GrGLRenderTarget*, const SkIRect& resolveRect, ResolveDirection,
                           bool invalidateReadBufferAfterBlit = false);

    // For loading a dynamic MSAA framebuffer when glCaps.canResolveSingleToMSAA() is false.
    // NOTE: If glCaps.framebufferResolvesMustBeFullSize() is also true, the drawBounds should be
    // equal to the proxy bounds. This is because the render pass will have to do a full size
    // resolve back into the single sample FBO when rendering is complete.
    void drawSingleIntoMSAAFBO(GrGLRenderTarget* rt, const SkIRect& drawBounds) {
        this->copySurfaceAsDraw(rt, true/*drawToMultisampleFBO*/, rt, drawBounds, drawBounds,
                                GrSamplerState::Filter::kNearest);
    }

    // The GrGLOpsRenderPass does not buffer up draws before submitting them to the gpu.
    // Thus this is the implementation of the clear call for the corresponding passthrough function
    // on GrGLOpsRenderPass.
    void clear(const GrScissorState&, std::array<float, 4> color, GrRenderTarget*,
               bool useMultisampleFBO, GrSurfaceOrigin);

    // The GrGLOpsRenderPass does not buffer up draws before submitting them to the gpu.
    // Thus this is the implementation of the clearStencil call for the corresponding passthrough
    // function on GrGLOpsrenderPass.
    void clearStencilClip(const GrScissorState&, bool insideStencilMask,
                          GrRenderTarget*, bool useMultisampleFBO, GrSurfaceOrigin);

    void beginCommandBuffer(GrGLRenderTarget*, bool useMultisampleFBO,
                            const SkIRect& bounds, GrSurfaceOrigin,
                            const GrOpsRenderPass::LoadAndStoreInfo& colorLoadStore,
                            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilLoadStore);

    void endCommandBuffer(GrGLRenderTarget*, bool useMultisampleFBO,
                          const GrOpsRenderPass::LoadAndStoreInfo& colorLoadStore,
                          const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilLoadStore);

    void invalidateBoundRenderTarget() {
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& colorFormat,
                                              SkISize dimensions, int numStencilSamples) override;

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless) override;

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

    bool precompileShader(const SkData& key, const SkData& data) override {
        return fProgramCache->precompileShader(this->getContext(), key, data);
    }

#if defined(GPU_TEST_UTILS)
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    const GrGLContext* glContextForTesting() const override { return &this->glContext(); }

    void resetShaderCacheForTesting() const override { fProgramCache->reset(); }
#endif

    void willExecute() override;

    void submit(GrOpsRenderPass* renderPass) override;

    [[nodiscard]] GrGLsync insertSync();
    bool testSync(GrGLsync);
    void deleteSync(GrGLsync);

    [[nodiscard]] std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership) override;
    void insertSemaphore(GrSemaphore* semaphore) override;
    void waitSemaphore(GrSemaphore* semaphore) override;

    std::optional<GrTimerQuery> startTimerQuery() override;
    uint64_t getTimerQueryResult(GrGLuint);

    void checkFinishedCallbacks() override;
    void finishOutstandingGpuWork() override;

    // Calls glGetError() until no errors are reported. Also looks for OOMs.
    void clearErrorsAndCheckForOOM();
    // Calls glGetError() once and returns the result. Also looks for an OOM.
    GrGLenum getErrorAndCheckForOOM();

    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    void bindFramebuffer(GrGLenum fboTarget, GrGLuint fboid);
    void deleteFramebuffer(GrGLuint fboid);

    void flushProgram(sk_sp<GrGLProgram>);

    // Version for programs that aren't GrGLProgram.
    void flushProgram(GrGLuint);

    // GrGLOpsRenderPass directly makes GL draws. GrGLGpu uses this notification to mark the
    // destination surface dirty if color writes are enabled.
    void didDrawTo(GrRenderTarget*);

private:
    GrGLGpu(std::unique_ptr<GrGLContext>, GrDirectContext*);

    // GrGpu overrides
    void endTimerQuery(const GrTimerQuery&) override;

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            skgpu::Mipmapped,
                                            GrProtected,
                                            std::string_view label) override;

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      skgpu::Mipmapped,
                                                      GrProtected) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void* data,
                                          size_t length) override;

    void onResetContext(uint32_t resetBits) override;

    void onResetTextureBindings() override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override;

    sk_sp<GrTexture> onCreateTexture(SkISize dimensions,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     skgpu::Budgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask,
                                     std::string_view label) override;
    sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                               const GrBackendFormat&,
                                               skgpu::Budgeted,
                                               skgpu::Mipmapped,
                                               GrProtected,
                                               const void* data,
                                               size_t dataSize) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType, GrAccessPattern) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrWrapOwnership,
                                          GrWrapCacheable,
                                          GrIOType) override;
    sk_sp<GrTexture> onWrapCompressedBackendTexture(const GrBackendTexture&,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    // Given a GL format return the index into the stencil format array on GrGLCaps to a
    // compatible stencil format, or negative if there is no compatible stencil format.
    int getCompatibleStencilIndex(GrGLFormat format);

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat& format) override {
        int idx = this->getCompatibleStencilIndex(GrBackendFormats::AsGLFormat(format));
        if (idx < 0) {
            return {};
        }
        return GrBackendFormats::MakeGL(GrGLFormatToEnum(this->glCaps().stencilFormats()[idx]),
                                        GR_GL_TEXTURE_NONE);
    }

    void onFBOChanged();

    // Returns whether the texture is successfully created. On success, a non-zero texture ID is
    // returned. On failure, zero is returned.
    // The texture is populated with |texels|, if it is non-null.
    // The texture parameters are cached in |initialTexParams|.
    GrGLuint createTexture(SkISize dimensions,
                           GrGLFormat,
                           GrGLenum target,
                           GrRenderable,
                           GrGLTextureParameters::SamplerOverriddenState*,
                           int mipLevelCount,
                           GrProtected isProtected,
                           std::string_view label);

    GrGLuint createCompressedTexture2D(SkISize dimensions,
                                       SkTextureCompressionType compression,
                                       GrGLFormat,
                                       skgpu::Mipmapped,
                                       GrProtected,
                                       GrGLTextureParameters::SamplerOverriddenState*);

    bool onReadPixels(GrSurface*,
                      SkIRect,
                      GrColorType surfaceColorType,
                      GrColorType dstColorType,
                      void*,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface*,
                       SkIRect,
                       GrColorType surfaceColorType,
                       GrColorType srcColorType,
                       const GrMipLevel[],
                       int mipLevelCount,
                       bool prepForTexSampling) override;

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override;

    bool onTransferPixelsTo(GrTexture*,
                            SkIRect,
                            GrColorType textureColorType,
                            GrColorType bufferColorType,
                            sk_sp<GrGpuBuffer>,
                            size_t offset,
                            size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface*,
                              SkIRect,
                              GrColorType surfaceColorType,
                              GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer>,
                              size_t offset) override;

    bool readOrTransferPixelsFrom(GrSurface*,
                                  SkIRect rect,
                                  GrColorType surfaceColorType,
                                  GrColorType dstColorType,
                                  void* offsetOrPtr,
                                  int rowWidthInPixels);

    // Unbinds xfer buffers from GL for operations that don't need them.
    // Before calling any variation of TexImage, TexSubImage, etc..., call this with
    // GrGpuBufferType::kXferCpuToGpu to ensure that the PIXEL_UNPACK_BUFFER is unbound.
    // Before calling ReadPixels and reading back into cpu memory call this with
    // GrGpuBufferType::kXferGpuToCpu to ensure that the PIXEL_PACK_BUFFER is unbound.
    void unbindXferBuffer(GrGpuBufferType type);

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) override;

    bool onRegenerateMipMapLevels(GrTexture*) override;

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter) override;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    void flushBlendAndColorWrite(const skgpu::BlendInfo&, const skgpu::Swizzle&);

    void addFinishedCallback(skgpu::AutoCallback callback, std::optional<GrTimerQuery>) override;

    GrOpsRenderPass* onGetOpsRenderPass(
            GrRenderTarget*,
            bool useMultisampleFBO,
            GrAttachment*,
            GrSurfaceOrigin,
            const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) override;

    void prepareSurfacesForBackendAccessAndStateUpdates(
            SkSpan<GrSurfaceProxy*> proxies,
            SkSurfaces::BackendSurfaceAccess access,
            const skgpu::MutableTextureState* newState) override;

    bool onSubmitToGpu(const GrSubmitInfo& info) override;

    bool copySurfaceAsDraw(GrSurface* dst, bool drawToMultisampleFBO, GrSurface* src,
                           const SkIRect& srcRect, const SkIRect& dstRect, GrSamplerState::Filter);
    void copySurfaceAsCopyTexSubImage(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                      const SkIPoint& dstPoint);
    bool copySurfaceAsBlitFramebuffer(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                      const SkIRect& dstRect, GrSamplerState::Filter);

    class ProgramCache : public GrThreadSafePipelineBuilder {
    public:
        ProgramCache(int runtimeProgramCacheSize);
        ~ProgramCache() override;

        void abandon();
        void reset();
        sk_sp<GrGLProgram> findOrCreateProgram(GrDirectContext*,
                                               const GrProgramInfo&);
        sk_sp<GrGLProgram> findOrCreateProgram(GrDirectContext*,
                                               const GrProgramDesc&,
                                               const GrProgramInfo&,
                                               Stats::ProgramCacheResult*);
        bool precompileShader(GrDirectContext*, const SkData& key, const SkData& data);

    private:
        struct Entry;

        sk_sp<GrGLProgram> findOrCreateProgramImpl(GrDirectContext*,
                                                   const GrProgramDesc&,
                                                   const GrProgramInfo&,
                                                   Stats::ProgramCacheResult*);

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkChecksum::Hash32(desc.asKey(), desc.keyLength());
            }
        };

        SkLRUCache<GrProgramDesc, std::unique_ptr<Entry>, DescHash> fMap;
    };

    void flushColorWrite(bool writeColor);
    void flushClearColor(std::array<float, 4>);

    // flushes the scissor. see the note on flushBoundTextureAndParams about
    // flushing the scissor after that function is called.
    void flushScissor(const GrScissorState& scissorState, int rtHeight, GrSurfaceOrigin rtOrigin) {
        this->flushScissorTest(GrScissorTest(scissorState.enabled()));
        if (scissorState.enabled()) {
            this->flushScissorRect(scissorState.rect(), rtHeight, rtOrigin);
        }
    }
    void flushScissorTest(GrScissorTest);

    void flushWindowRectangles(const GrWindowRectsState&, const GrGLRenderTarget*, GrSurfaceOrigin);
    void disableWindowRectangles();

    int numTextureUnits() const { return this->caps()->shaderCaps()->fMaxFragmentSamplers; }

    // Binds a texture to a target on the "scratch" texture unit to use for texture operations
    // other than usual draw flow (i.e. a GrGLProgram derived from a GrPipeline used to draw). It
    // ensures that such operations don't negatively interact with draws. The active texture unit
    // and the binding for 'target' will change.
    void bindTextureToScratchUnit(GrGLenum target, GrGLint textureID);

    void flushRenderTarget(GrGLRenderTarget*, bool useMultisampleFBO);

    void flushStencil(const GrStencilSettings&, GrSurfaceOrigin);
    void disableStencil();

    void flushConservativeRasterState(bool enable);

    void flushWireframeState(bool enable);

    void flushFramebufferSRGB(bool enable);

    // Uploads src data of a color type to the currently bound texture on the active texture unit.
    // The caller specifies color type that the texture is being used with, which may be different
    // than the src color type. This fails if the combination of texture format, texture color type,
    // and src data color type are not valid. No conversion is performed on the data before passing
    // it to GL. 'dstRect' must be the texture bounds if mipLevelCount is greater than 1.
    bool uploadColorTypeTexData(GrGLFormat textureFormat,
                                GrColorType textureColorType,
                                SkISize texDims,
                                GrGLenum target,
                                SkIRect dstRect,
                                GrColorType srcColorType,
                                const GrMipLevel texels[],
                                int mipLevelCount);

    // Uploads a constant color to a texture using the "default" format and color type. Overwrites
    // entire levels. Bit n in 'levelMask' indicates whether level n should be written. This
    // function doesn't know if MIP levels have been allocated, thus levelMask should not have bits
    // beyond the low bit set if the texture is not MIP mapped.
    bool uploadColorToTex(GrGLFormat textureFormat,
                          SkISize texDims,
                          GrGLenum target,
                          std::array<float, 4> color,
                          uint32_t levelMask);

    // Pushes data to the currently bound texture to the currently active unit. 'dstRect' must be
    // the texture bounds if mipLevelCount is greater than 1.
    void uploadTexData(SkISize dimensions,
                       GrGLenum target,
                       SkIRect dstRect,
                       GrGLenum externalFormat,
                       GrGLenum externalType,
                       size_t bpp,
                       const GrMipLevel texels[],
                       int mipLevelCount);

    // Helper for onCreateCompressedTexture. Compressed textures are read-only so we only use this
    // to populate a new texture. Returns false if we failed to create and upload the texture.
    bool uploadCompressedTexData(SkTextureCompressionType compressionType,
                                 GrGLFormat,
                                 SkISize dimensions,
                                 skgpu::Mipmapped,
                                 GrGLenum target,
                                 const void* data,
                                 size_t dataSize);

    // Calls one of various versions of renderBufferStorageMultisample.
    bool renderbufferStorageMSAA(const GrGLContext& ctx, int sampleCount, GrGLenum format,
                                 int width, int height);

    bool createRenderTargetObjects(const GrGLTexture::Desc&,
                                   int sampleCount,
                                   GrGLRenderTarget::IDs*);
    enum TempFBOTarget {
        kSrc_TempFBOTarget,
        kDst_TempFBOTarget
    };

    // Binds a surface as a FBO for copying, reading, or clearing. If the surface already owns an
    // FBO ID then that ID is bound. If not the surface is temporarily bound to a FBO and that FBO
    // is bound. This must be paired with a call to unbindSurfaceFBOForPixelOps().
    void bindSurfaceFBOForPixelOps(GrSurface* surface, int mipLevel, GrGLenum fboTarget,
                                   TempFBOTarget tempFBOTarget);

    // Must be called if bindSurfaceFBOForPixelOps was used to bind a surface for copying.
    void unbindSurfaceFBOForPixelOps(GrSurface* surface, int mipLevel, GrGLenum fboTarget);

#ifdef SK_ENABLE_DUMP_GPU
    void onDumpJSON(SkJSONWriter*) const override;
#endif

    bool createCopyProgram(GrTexture* srcTexture);
    bool createMipmapProgram(int progIdx);

    std::unique_ptr<GrGLContext> fGLContext;

    // GL program-related state
    sk_sp<ProgramCache>         fProgramCache;

    ///////////////////////////////////////////////////////////////////////////
    ///@name Caching of GL State
    ///@{
    int                         fHWActiveTextureUnitIdx;

    GrGLuint                    fHWProgramID;
    sk_sp<GrGLProgram>          fHWProgram;

    enum TriState {
        kNo_TriState,
        kYes_TriState,
        kUnknown_TriState
    };

    GrGLuint                    fTempSrcFBOID;
    GrGLuint                    fTempDstFBOID;

    GrGLuint                    fStencilClearFBOID;

    // last scissor / viewport scissor state seen by the GL.
    struct {
        TriState fEnabled;
        GrNativeRect fRect;
        void invalidate() {
            fEnabled = kUnknown_TriState;
            fRect.invalidate();
        }
    } fHWScissorSettings;

    class HWWindowRectsState {
    public:
        bool valid() const { return kInvalidSurfaceOrigin != fRTOrigin; }
        void invalidate() { fRTOrigin = kInvalidSurfaceOrigin; }
        bool knownDisabled() const { return this->valid() && !fWindowState.enabled(); }
        void setDisabled() {
            fRTOrigin = kTopLeft_GrSurfaceOrigin;
            fWindowState.setDisabled();
        }

        void set(GrSurfaceOrigin rtOrigin, int width, int height,
                 const GrWindowRectsState& windowState) {
            fRTOrigin = rtOrigin;
            fWidth = width;
            fHeight = height;
            fWindowState = windowState;
        }

        bool knownEqualTo(GrSurfaceOrigin rtOrigin, int width, int height,
                          const GrWindowRectsState& windowState) const {
            if (!this->valid()) {
                return false;
            }
            if (fWindowState.numWindows() &&
                (fRTOrigin != rtOrigin || fWidth != width || fHeight != height)) {
                return false;
            }
            return fWindowState == windowState;
        }

    private:
        static constexpr int kInvalidSurfaceOrigin = -1;

        int                  fRTOrigin;
        int                  fWidth;
        int                  fHeight;
        GrWindowRectsState   fWindowState;
    };
    HWWindowRectsState fHWWindowRectsState;

    GrNativeRect fHWViewport;

    /**
     * Tracks vertex attrib array state.
     */
    class HWVertexArrayState {
    public:
        HWVertexArrayState() : fCoreProfileVertexArray(nullptr) { this->invalidate(); }

        ~HWVertexArrayState() { delete fCoreProfileVertexArray; }

        void invalidate() {
            fBoundVertexArrayIDIsValid = false;
            fDefaultVertexArrayAttribState.invalidate();
            if (fCoreProfileVertexArray) {
                fCoreProfileVertexArray->invalidateCachedState();
            }
        }

        void notifyVertexArrayDelete(GrGLuint id) {
            if (fBoundVertexArrayIDIsValid && fBoundVertexArrayID == id) {
                // Does implicit bind to 0
                fBoundVertexArrayID = 0;
            }
        }

        void setVertexArrayID(GrGLGpu* gpu, GrGLuint arrayID) {
            if (!gpu->glCaps().vertexArrayObjectSupport()) {
                SkASSERT(0 == arrayID);
                return;
            }
            if (!fBoundVertexArrayIDIsValid || arrayID != fBoundVertexArrayID) {
                GR_GL_CALL(gpu->glInterface(), BindVertexArray(arrayID));
                fBoundVertexArrayIDIsValid = true;
                fBoundVertexArrayID = arrayID;
            }
        }

        /**
         * Binds the vertex array that should be used for internal draws, and returns its attrib
         * state. This binds the default VAO (ID=zero) unless we are on a core profile, in which
         * case we use a placeholder array instead.
         *
         * If an index buffer is provided, it will be bound to the vertex array. Otherwise the
         * index buffer binding will be left unchanged.
         *
         * The returned GrGLAttribArrayState should be used to set vertex attribute arrays.
         */
        GrGLAttribArrayState* bindInternalVertexArray(GrGLGpu*, const GrBuffer* ibuff = nullptr);

    private:
        GrGLuint             fBoundVertexArrayID;
        bool                 fBoundVertexArrayIDIsValid;

        // We return a non-const pointer to this from bindArrayAndBuffersToDraw when vertex array 0
        // is bound. However, this class is internal to GrGLGpu and this object never leaks out of
        // GrGLGpu.
        GrGLAttribArrayState fDefaultVertexArrayAttribState;

        // This is used when we're using a core profile.
        GrGLVertexArray*     fCoreProfileVertexArray;
    } fHWVertexArrayState;

    struct {
        GrGLenum                fGLTarget;
        GrGpuResource::UniqueID fBoundBufferUniqueID;
        bool                    fBufferZeroKnownBound;

        void invalidate() {
            fBoundBufferUniqueID.makeInvalid();
            fBufferZeroKnownBound = false;
        }
    }                                       fHWBufferState[kGrGpuBufferTypeCount];

    auto* hwBufferState(GrGpuBufferType type) {
        unsigned typeAsUInt = static_cast<unsigned>(type);
        SkASSERT(typeAsUInt < std::size(fHWBufferState));
        SkASSERT(type != GrGpuBufferType::kUniform);
        return &fHWBufferState[typeAsUInt];
    }

    enum class FlushType {
        kIfRequired,
        kForce,
    };

    // This calls glFlush if it is required for previous operations or kForce is passed.
    void flush(FlushType flushType = FlushType::kIfRequired);

    void setNeedsFlush() { fNeedsGLFlush = true; }

    struct {
        skgpu::BlendEquation fEquation;
        skgpu::BlendCoeff    fSrcCoeff;
        skgpu::BlendCoeff    fDstCoeff;
        SkPMColor4f          fConstColor;
        bool                 fConstColorValid;
        TriState             fEnabled;

        void invalidate() {
            fEquation = skgpu::BlendEquation::kIllegal;
            fSrcCoeff = skgpu::BlendCoeff::kIllegal;
            fDstCoeff = skgpu::BlendCoeff::kIllegal;
            fConstColorValid = false;
            fEnabled = kUnknown_TriState;
        }
    }                                       fHWBlendState;

    TriState                                fHWConservativeRasterEnabled;

    TriState                                fHWWireframeEnabled;

    GrStencilSettings                       fHWStencilSettings;
    GrSurfaceOrigin                         fHWStencilOrigin;
    TriState                                fHWStencilTestEnabled;

    TriState                                fHWWriteToColor;
    GrGpuResource::UniqueID                 fHWBoundRenderTargetUniqueID;
    bool                                    fHWBoundFramebufferIsMSAA;
    TriState                                fHWSRGBFramebuffer;

    class TextureUnitBindings {
    public:
        TextureUnitBindings() = default;
        TextureUnitBindings(const TextureUnitBindings&) = delete;
        TextureUnitBindings& operator=(const TextureUnitBindings&) = delete;

        GrGpuResource::UniqueID boundID(GrGLenum target) const;
        bool hasBeenModified(GrGLenum target) const;
        void setBoundID(GrGLenum target, GrGpuResource::UniqueID);
        void invalidateForScratchUse(GrGLenum target);
        void invalidateAllTargets(bool markUnmodified);

    private:
        struct TargetBinding {
            GrGpuResource::UniqueID fBoundResourceID;
            bool fHasBeenModified = false;
        };
        TargetBinding fTargetBindings[3];
    };
    skia_private::AutoTArray<TextureUnitBindings> fHWTextureUnitBindings;

    GrGLfloat fHWClearColor[4];

    GrGLuint fBoundDrawFramebuffer = 0;

    /** IDs for copy surface program. (3 sampler types) */
    struct {
        GrGLuint    fProgram = 0;
        GrGLint     fTextureUniform = 0;
        GrGLint     fTexCoordXformUniform = 0;
        GrGLint     fPosXformUniform = 0;
    }                                       fCopyPrograms[3];
    sk_sp<GrGLBuffer>                       fCopyProgramArrayBuffer;

    /** IDs for texture mipmap program. (4 filter configurations) */
    struct {
        GrGLuint    fProgram = 0;
        GrGLint     fTextureUniform = 0;
        GrGLint     fTexCoordXformUniform = 0;
    }                                       fMipmapPrograms[4];
    sk_sp<GrGLBuffer>                       fMipmapProgramArrayBuffer;

    static int TextureToCopyProgramIdx(GrTexture* texture);

    static int TextureSizeToMipmapProgramIdx(int width, int height) {
        const bool wide = (width > 1) && SkToBool(width & 0x1);
        const bool tall = (height > 1) && SkToBool(height & 0x1);
        return (wide ? 0x2 : 0x0) | (tall ? 0x1 : 0x0);
    }

    GrPrimitiveType fLastPrimitiveType;

    GrGLTextureParameters::ResetTimestamp fResetTimestampForTextureParameters = 0;

    class SamplerObjectCache;
    std::unique_ptr<SamplerObjectCache> fSamplerObjectCache;

    std::unique_ptr<GrGLOpsRenderPass> fCachedOpsRenderPass;

    std::unique_ptr<GrStagingBufferManager> fStagingBufferManager;

    GrGLFinishCallbacks fFinishCallbacks;

    // If we've called a command that requires us to call glFlush than this will be set to true
    // since we defer calling flush until submit time. When we call submitToGpu if this is true then
    // we call glFlush and reset this to false.
    bool fNeedsGLFlush = false;

    SkDEBUGCODE(bool fIsExecutingCommandBuffer_DebugOnly = false;)

    friend class GrGLPathRendering; // For accessing setTextureUnit.

    using INHERITED = GrGpu;
};

#endif
