/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGpuGL_DEFINED
#define GrGpuGL_DEFINED

#include "GrDrawState.h"
#include "GrGpu.h"
#include "GrGLContextInfo.h"
#include "GrGLIndexBuffer.h"
#include "GrGLIRect.h"
#include "GrGLStencilBuffer.h"
#include "GrGLTexture.h"
#include "GrGLVertexBuffer.h"

class GrGpuGL : public GrGpu {
public:
    virtual ~GrGpuGL();

    const GrGLInterface* glInterface() const { 
        return fGLContextInfo.interface();
    }
    GrGLBinding glBinding() const { return fGLContextInfo.binding(); }
    GrGLVersion glVersion() const { return fGLContextInfo.version(); }
    GrGLSLGeneration glslGeneration() const {
        return fGLContextInfo.glslGeneration();
    }

    // GrGpu overrides
    virtual GrPixelConfig preferredReadPixelsConfig(GrPixelConfig config)
                                                            const SK_OVERRIDE;
    virtual GrPixelConfig preferredWritePixelsConfig(GrPixelConfig config)
                                                            const SK_OVERRIDE;
    virtual bool readPixelsWillPayForYFlip(
                                    GrRenderTarget* renderTarget,
                                    int left, int top,
                                    int width, int height,
                                    GrPixelConfig config,
                                    size_t rowBytes) const SK_OVERRIDE;
    virtual bool fullReadPixelsIsFasterThanPartial() const SK_OVERRIDE;

    virtual bool canPreserveReadWriteUnpremulPixels() SK_OVERRIDE;

protected:
    GrGpuGL(const GrGLContextInfo& ctxInfo);

    struct {
        size_t                  fVertexOffset;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
        bool                    fArrayPtrsDirty;
    } fHWGeometryState;

    struct AAState {
        bool fMSAAEnabled;
        bool fSmoothLineEnabled;
    } fHWAAState;

    enum UnpremulConversion {
        kUpOnWrite_DownOnRead_UnpremulConversion,
        kDownOnWrite_UpOnRead_UnpremulConversion
    } fUnpremulConversion;

    GrDrawState fHWDrawState;
    bool        fHWStencilClip;

    // As flush of GL state proceeds it updates fHDrawState
    // to reflect the new state. Later parts of the state flush
    // may perform cascaded changes but cannot refer to fHWDrawState.
    // These code paths can refer to the dirty flags. Subclass should
    // call resetDirtyFlags after its flush is complete
    struct {
        bool fRenderTargetChanged : 1;
        int  fTextureChangedMask;
    } fDirtyFlags;
    GR_STATIC_ASSERT(8 * sizeof(int) >= GrDrawState::kNumStages);

    // clears the dirty flags
    void resetDirtyFlags();

    // last scissor / viewport scissor state seen by the GL.
    struct {
        bool        fScissorEnabled;
        GrGLIRect   fScissorRect;
        GrGLIRect   fViewportRect;
    } fHWBounds;

    const GrGLCaps& glCaps() const { return fGLContextInfo.caps(); }

    // GrGpu overrides
    virtual void onResetContext() SK_OVERRIDE;

    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes);
    virtual GrVertexBuffer* onCreateVertexBuffer(uint32_t size,
                                                 bool dynamic);
    virtual GrIndexBuffer* onCreateIndexBuffer(uint32_t size,
                                               bool dynamic);
    virtual GrTexture* onCreatePlatformTexture(const GrPlatformTextureDesc& desc) SK_OVERRIDE;
    virtual GrRenderTarget* onCreatePlatformRenderTarget(const GrPlatformRenderTargetDesc& desc) SK_OVERRIDE;
    virtual bool createStencilBufferForRenderTarget(GrRenderTarget* rt,
                                                    int width, int height);
    virtual bool attachStencilBufferToRenderTarget(GrStencilBuffer* sb,
                                                   GrRenderTarget* rt);

    virtual void onClear(const GrIRect* rect, GrColor color);

    virtual void onForceRenderTargetFlush();

    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, 
                              int width, int height,
                              GrPixelConfig, 
                              void* buffer,
                              size_t rowBytes,
                              bool invertY) SK_OVERRIDE;

    virtual void onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes) SK_OVERRIDE;

    virtual void onResolveRenderTarget(GrRenderTarget* target) SK_OVERRIDE;


    virtual void onGpuDrawIndexed(GrPrimitiveType type,
                                  uint32_t startVertex,
                                  uint32_t startIndex,
                                  uint32_t vertexCount,
                                  uint32_t indexCount);
    virtual void onGpuDrawNonIndexed(GrPrimitiveType type,
                                     uint32_t vertexCount,
                                     uint32_t numVertices);
    virtual void enableScissoring(const GrIRect& rect);
    virtual void disableScissor();

    virtual void clearStencil();
    virtual void clearStencilClip(const GrIRect& rect, bool insideClip);

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    // binds appropriate vertex and index buffers, also returns any extra
    // extra verts or indices to offset by.
    void setBuffers(bool indexed,
                    int* extraVertexOffset,
                    int* extraIndexOffset);

    // flushes state that is common to fixed and programmable GL
    // dither
    // line smoothing
    // texture binding
    // sampler state (filtering, tiling)
    // FBO binding
    // line width
    bool flushGLStateCommon(GrPrimitiveType type);

    // Subclasses should call this to flush the blend state.
    // The params should be the final coeffecients to apply
    // (after any blending optimizations or dual source blending considerations
    // have been accounted for).
    void flushBlend(GrPrimitiveType type,
                    GrBlendCoeff srcCoeff,
                    GrBlendCoeff dstCoeff);

    bool hasExtension(const char* ext) const {
        return fGLContextInfo.hasExtension(ext);
    }

    const GrGLContextInfo& glContextInfo() const { return fGLContextInfo; }

    // adjusts texture matrix to account for orientation
    static void AdjustTextureMatrix(const GrGLTexture* texture,
                                    GrSamplerState::SampleMode mode,
                                    GrMatrix* matrix);

    // subclass may try to take advantage of identity tex matrices.
    // This helper determines if matrix will be identity after all
    // adjustments are applied.
    static bool TextureMatrixIsIdentity(const GrGLTexture* texture,
                                        const GrSamplerState& sampler);

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

private:
    // Inits GrDrawTarget::Caps, sublcass may enable additional caps.
    void initCaps();

    void initFSAASupport();

    // determines valid stencil formats
    void initStencilFormats();

    // notify callbacks to update state tracking when related
    // objects are bound to GL or deleted outside of the class
    void notifyVertexBufferBind(const GrGLVertexBuffer* buffer);
    void notifyVertexBufferDelete(const GrGLVertexBuffer* buffer);
    void notifyIndexBufferBind(const GrGLIndexBuffer* buffer);
    void notifyIndexBufferDelete(const GrGLIndexBuffer* buffer);
    void notifyTextureDelete(GrGLTexture* texture);
    void notifyRenderTargetDelete(GrRenderTarget* renderTarget);

    void setSpareTextureUnit();

    // bound is region that may be modified and therefore has to be resolved.
    // NULL means whole target. Can be an empty rect.
    void flushRenderTarget(const GrIRect* bound);
    void flushStencil();
    void flushAAState(GrPrimitiveType type);

    bool configToGLFormats(GrPixelConfig config,
                           bool getSizedInternal,
                           GrGLenum* internalFormat,
                           GrGLenum* externalFormat,
                           GrGLenum* externalType);
    // helper for onCreateTexture and writeTexturePixels
    bool uploadTexData(const GrGLTexture::Desc& desc,
                       bool isNewTexture,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const void* data,
                       size_t rowBytes);

    bool createRenderTargetObjects(int width, int height,
                                   GrGLuint texID,
                                   GrGLRenderTarget::Desc* desc);

    void fillInConfigRenderableTable();

    friend class GrGLVertexBuffer;
    friend class GrGLIndexBuffer;
    friend class GrGLTexture;
    friend class GrGLRenderTarget;

    GrGLContextInfo fGLContextInfo;

    // we want to clear stencil buffers when they are created. We want to clear
    // the entire buffer even if it is larger than the color attachment. We
    // attach it to this fbo with no color attachment to do the initial clear.
    GrGLuint fStencilClearFBO;

    bool fHWBlendDisabled;

    int fActiveTextureUnitIdx;

    // we record what stencil format worked last time to hopefully exit early
    // from our loop that tries stencil formats and calls check fb status.
    int fLastSuccessfulStencilFmtIdx;

    enum CanPreserveUnpremulRoundtrip {
        kUnknown_CanPreserveUnpremulRoundtrip,
        kNo_CanPreserveUnpremulRoundtrip,
        kYes_CanPreserveUnpremulRoundtrip,
    } fCanPreserveUnpremulRoundtrip;

    bool fPrintedCaps;

    typedef GrGpu INHERITED;
};

#endif

