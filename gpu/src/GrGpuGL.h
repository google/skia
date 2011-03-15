/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGpuGL_DEFINED
#define GrGpuGL_DEFINED

#include "GrGpu.h"
#include "GrGLConfig.h"
#include "GrGLIRect.h"
#include "GrGLTexture.h"

#include "GrGLVertexBuffer.h"
#include "GrGLIndexBuffer.h"

class GrGpuGL : public GrGpu {
public:
            GrGpuGL();
    virtual ~GrGpuGL();

protected:
    struct {
        size_t                  fVertexOffset;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
        bool                    fArrayPtrsDirty;
    } fHWGeometryState;

    DrState   fHWDrawState;
    bool      fHWStencilClip;

    // As flush of GL state proceeds it updates fHDrawState
    // to reflect the new state. Later parts of the state flush
    // may perform cascaded changes but cannot refer to fHWDrawState.
    // These code paths can refer to the dirty flags. Subclass should
    // call resetDirtyFlags after its flush is complete
    struct {
        bool fRenderTargetChanged : 1;
        int  fTextureChangedMask;
    } fDirtyFlags;
    GR_STATIC_ASSERT(8 * sizeof(int) >= kNumStages);

    // clears the dirty flags
    void resetDirtyFlags();

    // last scissor / viewport scissor state seen by the GL.
    struct {
        bool        fScissorEnabled;
        GrGLIRect   fScissorRect;
        GrGLIRect   fViewportRect;
    } fHWBounds;

    // GrGpu overrides
    // overrides from GrGpu
    virtual void resetContext();

    virtual GrTexture* createTextureHelper(const TextureDesc& desc,
                                           const void* srcData,
                                           size_t rowBytes);
    virtual GrVertexBuffer* createVertexBufferHelper(uint32_t size,
                                                     bool dynamic);
    virtual GrIndexBuffer* createIndexBufferHelper(uint32_t size,
                                                   bool dynamic);

    virtual GrRenderTarget* createPlatformRenderTargetHelper(
                                                 intptr_t platformRenderTarget,
                                                 int stencilBits,
                                                 int width, int height);

    virtual GrRenderTarget* createRenderTargetFrom3DApiStateHelper();

    virtual void eraseColorHelper(GrColor color);

    virtual void forceRenderTargetFlushHelper();

    virtual bool readPixelsHelper(int left, int top, int width, int height,
                                  GrTexture::PixelConfig, void* buffer);

    virtual void drawIndexedHelper(GrPrimitiveType type,
                                   uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount);
    virtual void drawNonIndexedHelper(GrPrimitiveType type,
                                      uint32_t vertexCount,
                                      uint32_t numVertices);
    virtual void flushScissor(const GrIRect* rect);
    void eraseStencil(uint32_t value, uint32_t mask);
    virtual void eraseStencilClip(const GrIRect& rect);

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
    // blend func
    // texture binding
    // sampler state (filtering, tiling)
    // FBO binding
    // line width
    bool flushGLStateCommon(GrPrimitiveType type);

    // adjusts texture matrix to account for orientation, size, and npotness
    static void AdjustTextureMatrix(const GrGLTexture* texture,
                                    GrSamplerState::SampleMode mode,
                                    GrMatrix* matrix);

    // subclass may try to take advantage of identity tex matrices.
    // This helper determines if matrix will be identity after all
    // adjustments are applied.
    static bool TextureMatrixIsIdentity(const GrGLTexture* texture,
                                        const GrSamplerState& sampler);

    static bool BlendCoefReferencesConstant(GrBlendCoeff coeff);

private:
    // notify callbacks to update state tracking when related
    // objects are bound to GL or deleted outside of the class
    void notifyVertexBufferBind(const GrGLVertexBuffer* buffer);
    void notifyVertexBufferDelete(const GrGLVertexBuffer* buffer);
    void notifyIndexBufferBind(const GrGLIndexBuffer* buffer);
    void notifyIndexBufferDelete(const GrGLIndexBuffer* buffer);
    void notifyTextureDelete(GrGLTexture* texture);
    void notifyRenderTargetDelete(GrRenderTarget* renderTarget);

    void setSpareTextureUnit();

    void flushRenderTarget();
    void flushStencil();
    void resolveTextureRenderTarget(GrGLTexture* texture);

    bool canBeTexture(GrTexture::PixelConfig config,
                      GLenum* internalFormat,
                      GLenum* format,
                      GLenum* type);
    bool fboInternalFormat(GrTexture::PixelConfig config, GLenum* format);

    friend class GrGLVertexBuffer;
    friend class GrGLIndexBuffer;
    friend class GrGLTexture;
    friend class GrGLRenderTarget;

    bool fHWBlendDisabled;

    GLuint fAASamples[4];
    enum {
        kNone_MSFBO = 0,
        kDesktop_MSFBO,
        kApple_MSFBO,
        kIMG_MSFBO
    } fMSFBOType;

    // Do we have stencil wrap ops.
    bool fHasStencilWrap;

    // ES requires an extension to support RGBA8 in RenderBufferStorage
    bool fRGBA8Renderbuffer;

    int fActiveTextureUnitIdx;

    typedef GrGpu INHERITED;
};

#endif
