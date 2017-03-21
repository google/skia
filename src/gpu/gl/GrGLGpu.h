/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGpu_DEFINED
#define GrGLGpu_DEFINED

#include "GrGLContext.h"
#include "GrGLIRect.h"
#include "GrGLPathRendering.h"
#include "GrGLProgram.h"
#include "GrGLRenderTarget.h"
#include "GrGLStencilAttachment.h"
#include "GrGLTexture.h"
#include "GrGLVertexArray.h"
#include "GrGpu.h"
#include "GrTexturePriv.h"
#include "GrWindowRectsState.h"
#include "GrXferProcessor.h"
#include "SkLRUCache.h"
#include "SkTArray.h"
#include "SkTypes.h"

class GrGLBuffer;
class GrPipeline;
class GrNonInstancedMesh;
class GrSwizzle;

namespace gr_instanced { class GLInstancedRendering; }

#ifdef SK_DEBUG
#define PROGRAM_CACHE_STATS
#endif

class GrGLGpu final : public GrGpu {
public:
    static GrGpu* Create(GrBackendContext backendContext, const GrContextOptions& options,
                         GrContext* context);
    ~GrGLGpu() override;

    void disconnect(DisconnectType) override;

    const GrGLContext& glContext() const { return *fGLContext; }

    const GrGLInterface* glInterface() const { return fGLContext->interface(); }
    const GrGLContextInfo& ctxInfo() const { return *fGLContext; }
    GrGLStandard glStandard() const { return fGLContext->standard(); }
    GrGLVersion glVersion() const { return fGLContext->version(); }
    GrGLSLGeneration glslGeneration() const { return fGLContext->glslGeneration(); }
    const GrGLCaps& glCaps() const { return *fGLContext->caps(); }

    GrGLPathRendering* glPathRendering() {
        SkASSERT(glCaps().shaderCaps()->pathRenderingSupport());
        return static_cast<GrGLPathRendering*>(pathRendering());
    }

    // Used by GrGLProgram to configure OpenGL state.
    void bindTexture(int unitIdx, const GrSamplerParams& params, bool allowSRGBInputs,
                     GrGLTexture* texture);

    void bindTexelBuffer(int unitIdx, GrPixelConfig, GrGLBuffer*);

    void bindImageStorage(int unitIdx, GrIOType, GrGLTexture *);

    void generateMipmaps(const GrSamplerParams& params, bool allowSRGBInputs, GrGLTexture* texture);

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override;

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override;

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
    GrGLenum bindBuffer(GrBufferType type, const GrBuffer*);

    // Called by GrGLBuffer after its buffer object has been destroyed.
    void notifyBufferReleased(const GrGLBuffer*);

    // The GrGLGpuCommandBuffer does not buffer up draws before submitting them to the gpu.
    // Thus this is the implementation of the draw call for the corresponding passthrough function
    // on GrGLGpuCommandBuffer.
    void draw(const GrPipeline&,
              const GrPrimitiveProcessor&,
              const GrMesh*,
              int meshCount);

    // The GrGLGpuCommandBuffer does not buffer up draws before submitting them to the gpu.
    // Thus this is the implementation of the clear call for the corresponding passthrough function
    // on GrGLGpuCommandBuffer.
    void clear(const GrFixedClip&, GrColor, GrRenderTarget*);

    // The GrGLGpuCommandBuffer does not buffer up draws before submitting them to the gpu.
    // Thus this is the implementation of the clearStencil call for the corresponding passthrough
    // function on GrGLGpuCommandBuffer.
    void clearStencilClip(const GrFixedClip&, bool insideStencilMask, GrRenderTarget*);

    const GrGLContext* glContextForTesting() const override {
        return &this->glContext();
    }

    void clearStencil(GrRenderTarget*) override;

    GrGpuCommandBuffer* createCommandBuffer(
            const GrGpuCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuCommandBuffer::LoadAndStoreInfo& stencilInfo) override;

    void invalidateBoundRenderTarget() {
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                int width,
                                                                int height) override;

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config,
                                                    bool isRenderTarget = false) override;
    bool isTestingOnlyBackendTexture(GrBackendObject) const override;
    void deleteTestingOnlyBackendTexture(GrBackendObject, bool abandonTexture) override;

    void resetShaderCacheForTesting() const override;

    void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore() override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;

    void deleteSync(GrGLsync) const;

    void flush() override;

private:
    GrGLGpu(GrGLContext* ctx, GrContext* context);

    // GrGpu overrides
    void onResetContext(uint32_t resetBits) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override;

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                               const SkTArray<GrMipLevel>& texels) override;
    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                         SkBudgeted budgeted,
                                         const SkTArray<GrMipLevel>& texels) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType intendedType, GrAccessPattern,
                             const void* data) override;
    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) override;
    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&) override;

    gr_instanced::InstancedRendering* onCreateInstancedRendering() override;

    // Given a GrPixelConfig return the index into the stencil format array on GrGLCaps to a
    // compatible stencil format, or negative if there is no compatible stencil format.
    int getCompatibleStencilIndex(GrPixelConfig config);


    // Returns whether the texture is successfully created. On success, the
    // result is stored in |info|.
    // The texture is populated with |texels|, if it exists.
    // The texture parameters are cached in |initialTexParams|.
    bool createTextureImpl(const GrSurfaceDesc& desc, GrGLTextureInfo* info,
                           bool renderTarget, GrGLTexture::TexParams* initialTexParams,
                           const SkTArray<GrMipLevel>& texels);

    bool onIsACopyNeededForTextureParams(GrTextureProxy*, const GrSamplerParams&,
                                         GrTextureProducer::CopyParams*,
                                         SkScalar scaleAdjust[2]) const override;

    // Checks whether glReadPixels can be called to get pixel values in readConfig from the
    // render target.
    bool readPixelsSupported(GrRenderTarget* target, GrPixelConfig readConfig);

    // Checks whether glReadPixels can be called to get pixel values in readConfig from a
    // render target that has renderTargetConfig. This may have to create a temporary
    // render target and thus is less preferable than the variant that takes a render target.
    bool readPixelsSupported(GrPixelConfig renderTargetConfig, GrPixelConfig readConfig);

    // Checks whether glReadPixels can be called to get pixel values in readConfig from a
    // render target that has the same config as surfaceForConfig. Calls one of the the two
    // variations above, depending on whether the surface is a render target or not.
    bool readPixelsSupported(GrSurface* surfaceForConfig, GrPixelConfig readConfig);

    bool onReadPixels(GrSurface*,
                      int left, int top,
                      int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface*,
                       int left, int top, int width, int height,
                       GrPixelConfig config,
                       const SkTArray<GrMipLevel>& texels) override;

    bool onTransferPixels(GrSurface*,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override;

    void onResolveRenderTarget(GrRenderTarget* target) override;

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override;

    void onQueryMultisampleSpecs(GrRenderTarget*, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    void setTextureSwizzle(int unitIdx, GrGLenum target, const GrGLenum swizzle[]);

    // Flushes state from GrPipeline to GL. Returns false if the state couldn't be set.
    // willDrawPoints must be true if point primitives will be rendered after setting the GL state.
    bool flushGLState(const GrPipeline&, const GrPrimitiveProcessor&, bool willDrawPoints);

    // Sets up vertex attribute pointers and strides. On return indexOffsetInBytes gives the offset
    // an into the index buffer. It does not account for vertices.startIndex() but rather the start
    // index is relative to the returned offset.
    void setupGeometry(const GrPrimitiveProcessor&,
                       const GrNonInstancedMesh& mesh,
                       size_t* indexOffsetInBytes);

    void flushBlend(const GrXferProcessor::BlendInfo& blendInfo, const GrSwizzle&);

    bool hasExtension(const char* ext) const { return fGLContext->hasExtension(ext); }

    bool copySurfaceAsDraw(GrSurface* dst,
                           GrSurface* src,
                           const SkIRect& srcRect,
                           const SkIPoint& dstPoint);
    void copySurfaceAsCopyTexSubImage(GrSurface* dst,
                                      GrSurface* src,
                                      const SkIRect& srcRect,
                                      const SkIPoint& dstPoint);
    bool copySurfaceAsBlitFramebuffer(GrSurface* dst,
                                      GrSurface* src,
                                      const SkIRect& srcRect,
                                      const SkIPoint& dstPoint);
    bool generateMipmap(GrGLTexture* texture, bool gammaCorrect);

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

    class ProgramCache : public ::SkNoncopyable {
    public:
        ProgramCache(GrGLGpu* gpu);
        ~ProgramCache();

        void abandon();
        GrGLProgram* refProgram(const GrGLGpu*, const GrPipeline&, const GrPrimitiveProcessor&,
                                bool hasPointSize);

    private:
        // We may actually have kMaxEntries+1 shaders in the GL context because we create a new
        // shader before evicting from the cache.
        static const int kMaxEntries = 128;

        struct Entry;

        // binary search for entry matching desc. returns index into fEntries that matches desc or ~
        // of the index of where it should be inserted.
        int search(const GrProgramDesc& desc) const;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<GrProgramDesc, std::unique_ptr<Entry>, DescHash> fMap;

        GrGLGpu*                    fGpu;
#ifdef PROGRAM_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
        int                         fHashMisses; // cache hit but hash table missed
#endif
    };

    void flushColorWrite(bool writeColor);
    void flushDrawFace(GrDrawFace face);

    // flushes the scissor. see the note on flushBoundTextureAndParams about
    // flushing the scissor after that function is called.
    void flushScissor(const GrScissorState&,
                      const GrGLIRect& rtViewport,
                      GrSurfaceOrigin rtOrigin);

    // disables the scissor
    void disableScissor();

    void flushWindowRectangles(const GrWindowRectsState&, const GrGLRenderTarget*);
    void disableWindowRectangles();

    void initFSAASupport();

    // determines valid stencil formats
    void initStencilFormats();

    // sets a texture unit to use for texture operations other than binding a texture to a program.
    // ensures that such operations don't negatively interact with tracking bound textures.
    void setScratchTextureUnit();

    // bounds is region that may be modified.
    // nullptr means whole target. Can be an empty rect.
    void flushRenderTarget(GrGLRenderTarget*, const SkIRect* bounds, bool disableSRGB = false);

    // Need not be called if flushRenderTarget is used.
    void flushViewport(const GrGLIRect&);

    void flushStencil(const GrStencilSettings&);
    void disableStencil();

    // rt is used only if useHWAA is true.
    void flushHWAAState(GrRenderTarget* rt, bool useHWAA, bool stencilEnabled);

    void flushMinSampleShading(float minSampleShading);

    void flushFramebufferSRGB(bool enable);

    // helper for onCreateTexture and writeTexturePixels
    enum UploadType {
        kNewTexture_UploadType,    // we are creating a new texture
        kWrite_UploadType,         // we are using TexSubImage2D to copy data to an existing texture
        kTransfer_UploadType,      // we are using a transfer buffer to copy data
    };
    bool uploadTexData(const GrSurfaceDesc& desc,
                       GrGLenum target,
                       UploadType uploadType,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const SkTArray<GrMipLevel>& texels);

    // helper for onCreateCompressedTexture. If width and height are
    // set to -1, then this function will use desc.fWidth and desc.fHeight
    // for the size of the data. The isNewTexture flag should be set to true
    // whenever a new texture needs to be created. Otherwise, we assume that
    // the texture is already in GPU memory and that it's going to be updated
    // with new data.
    bool uploadCompressedTexData(const GrSurfaceDesc& desc,
                                 GrGLenum target,
                                 const SkTArray<GrMipLevel>& texels,
                                 UploadType uploadType = kNewTexture_UploadType,
                                 int left = 0, int top = 0,
                                 int width = -1, int height = -1);

    bool createRenderTargetObjects(const GrSurfaceDesc&, const GrGLTextureInfo& texInfo,
                                   GrGLRenderTarget::IDDesc*);

    enum TempFBOTarget {
        kSrc_TempFBOTarget,
        kDst_TempFBOTarget
    };

    // Binds a surface as a FBO for copying or reading. If the surface already owns an FBO ID then
    // that ID is bound. If not the surface is temporarily bound to a FBO and that FBO is bound.
    // This must be paired with a call to unbindSurfaceFBOForPixelOps().
    void bindSurfaceFBOForPixelOps(GrSurface* surface, GrGLenum fboTarget, GrGLIRect* viewport,
                                   TempFBOTarget tempFBOTarget);

    // Must be called if bindSurfaceFBOForPixelOps was used to bind a surface for copying.
    void unbindTextureFBOForPixelOps(GrGLenum fboTarget, GrSurface* surface);

    sk_sp<GrGLContext>          fGLContext;

    bool createCopyProgram(GrTexture* srcTexture);
    bool createMipmapProgram(int progIdx);
    bool createWireRectProgram();

    // GL program-related state
    ProgramCache*               fProgramCache;

    ///////////////////////////////////////////////////////////////////////////
    ///@name Caching of GL State
    ///@{
    int                         fHWActiveTextureUnitIdx;
    GrGLuint                    fHWProgramID;

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
        TriState    fEnabled;
        GrGLIRect   fRect;
        void invalidate() {
            fEnabled = kUnknown_TriState;
            fRect.invalidate();
        }
    } fHWScissorSettings;

    class {
    public:
        bool valid() const { return kInvalidSurfaceOrigin != fRTOrigin; }
        void invalidate() { fRTOrigin = kInvalidSurfaceOrigin; }
        bool knownDisabled() const { return this->valid() && !fWindowState.enabled(); }
        void setDisabled() {
            fRTOrigin = kDefault_GrSurfaceOrigin;
            fWindowState.setDisabled();
        }

        void set(GrSurfaceOrigin rtOrigin, const GrGLIRect& viewport,
                 const GrWindowRectsState& windowState) {
            fRTOrigin = rtOrigin;
            fViewport = viewport;
            fWindowState = windowState;
        }

        bool knownEqualTo(GrSurfaceOrigin rtOrigin, const GrGLIRect& viewport,
                          const GrWindowRectsState& windowState) const {
            if (!this->valid()) {
                return false;
            }
            if (fWindowState.numWindows() && (fRTOrigin != rtOrigin || fViewport != viewport)) {
                return false;
            }
            return fWindowState == windowState;
        }

    private:
        enum { kInvalidSurfaceOrigin = -1 };

        int                  fRTOrigin;
        GrGLIRect            fViewport;
        GrWindowRectsState   fWindowState;
    } fHWWindowRectsState;

    GrGLIRect                   fHWViewport;

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
         * case we use a dummy array instead.
         *
         * If an index buffer is privided, it will be bound to the vertex array. Otherwise the
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
    }                                       fHWVertexArrayState;

    struct {
        GrGLenum                fGLTarget;
        GrGpuResource::UniqueID fBoundBufferUniqueID;
        bool                    fBufferZeroKnownBound;

        void invalidate() {
            fBoundBufferUniqueID.makeInvalid();
            fBufferZeroKnownBound = false;
        }
    }                                       fHWBufferState[kGrBufferTypeCount];

    struct {
        GrBlendEquation fEquation;
        GrBlendCoeff    fSrcCoeff;
        GrBlendCoeff    fDstCoeff;
        GrColor         fConstColor;
        bool            fConstColorValid;
        TriState        fEnabled;

        void invalidate() {
            fEquation = static_cast<GrBlendEquation>(-1);
            fSrcCoeff = static_cast<GrBlendCoeff>(-1);
            fDstCoeff = static_cast<GrBlendCoeff>(-1);
            fConstColorValid = false;
            fEnabled = kUnknown_TriState;
        }
    }                                       fHWBlendState;

    TriState fMSAAEnabled;

    GrStencilSettings                       fHWStencilSettings;
    TriState                                fHWStencilTestEnabled;


    GrDrawFace                              fHWDrawFace;
    TriState                                fHWWriteToColor;
    GrGpuResource::UniqueID                 fHWBoundRenderTargetUniqueID;
    TriState                                fHWSRGBFramebuffer;
    SkTArray<GrGpuResource::UniqueID, true> fHWBoundTextureUniqueIDs;

    struct Image {
        GrGpuResource::UniqueID fTextureUniqueID;
        GrIOType                fIOType;
    };
    SkTArray<Image, true>                   fHWBoundImageStorages;

    struct BufferTexture {
        BufferTexture() : fTextureID(0), fKnownBound(false),
                          fAttachedBufferUniqueID(SK_InvalidUniqueID),
                          fSwizzle(GrSwizzle::RGBA()) {}

        GrGLuint                fTextureID;
        bool                    fKnownBound;
        GrPixelConfig           fTexelConfig;
        GrGpuResource::UniqueID fAttachedBufferUniqueID;
        GrSwizzle               fSwizzle;
    };

    SkTArray<BufferTexture, true>           fHWBufferTextures;
    int                                     fHWMaxUsedBufferTextureUnit;

    // EXT_raster_multisample.
    TriState                                fHWRasterMultisampleEnabled;
    int                                     fHWNumRasterSamples;
    ///@}

    /** IDs for copy surface program. (4 sampler types) */
    struct {
        GrGLuint    fProgram;
        GrGLint     fTextureUniform;
        GrGLint     fTexCoordXformUniform;
        GrGLint     fPosXformUniform;
    }                                       fCopyPrograms[4];
    sk_sp<GrGLBuffer>                       fCopyProgramArrayBuffer;

    /** IDs for texture mipmap program. (4 filter configurations) */
    struct {
        GrGLuint    fProgram;
        GrGLint     fTextureUniform;
        GrGLint     fTexCoordXformUniform;
    }                                       fMipmapPrograms[4];
    sk_sp<GrGLBuffer>                       fMipmapProgramArrayBuffer;

    struct {
        GrGLuint    fProgram;
        GrGLint     fColorUniform;
        GrGLint     fRectUniform;
    }                                       fWireRectProgram;
    sk_sp<GrGLBuffer>                       fWireRectArrayBuffer;

    static int TextureToCopyProgramIdx(GrTexture* texture) {
        switch (texture->texturePriv().samplerType()) {
            case kTexture2DSampler_GrSLType:
                return 0;
            case kITexture2DSampler_GrSLType:
                return 1;
            case kTexture2DRectSampler_GrSLType:
                return 2;
            case kTextureExternalSampler_GrSLType:
                return 3;
            default:
                SkFAIL("Unexpected samper type");
                return 0;
        }
    }

    static int TextureSizeToMipmapProgramIdx(int width, int height) {
        const bool wide = (width > 1) && SkToBool(width & 0x1);
        const bool tall = (height > 1) && SkToBool(height & 0x1);
        return (wide ? 0x2 : 0x0) | (tall ? 0x1 : 0x0);
    }

    float                                   fHWMinSampleShading;

    typedef GrGpu INHERITED;
    friend class GrGLPathRendering; // For accessing setTextureUnit.
    friend class gr_instanced::GLInstancedRendering; // For accessing flushGLState.
};

#endif
