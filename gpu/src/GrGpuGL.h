/*
    Copyright 2010 Google Inc.

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
#include "GrGLTexture.h"

#include "GrGLVertexBuffer.h"
#include "GrGLIndexBuffer.h"

class GrGpuGL : public GrGpu {
public:
            GrGpuGL();
    virtual ~GrGpuGL();

    // overrides from GrGpu
    virtual void resetContext();

    virtual GrTexture* createTexture(const TextureDesc& desc,
                                     const void* srcData, size_t rowBytes);
    virtual GrVertexBuffer* createVertexBuffer(uint32_t size, bool dynamic);
    virtual GrIndexBuffer* createIndexBuffer(uint32_t size, bool dynamic);

    virtual GrRenderTarget* createPlatformRenderTarget(
                                                 intptr_t platformRenderTarget,
                                                 int width, int height);

    virtual GrRenderTarget* defaultRenderTarget();

    virtual void setDefaultRenderTargetSize(uint32_t width, uint32_t height);

    virtual void eraseColor(GrColor color);

    virtual void forceRenderTargetFlush();

    virtual bool readPixels(int left, int top, int width, int height,
                            GrTexture::PixelConfig, void* buffer);

    /**
     * Gets the struct containing the GL extensions for the context
     * underlying the GrGpuGL
     *
     * @param struct containing extension function pointers
     */
    const GrGLExts& extensions() { return fExts; }

protected:
    struct {
        const void*
        fPositionPtr;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
    } fHWGeometryState;

    DrawState   fHWDrawState;
    bool        fHWStencilClip;

    virtual void drawIndexedHelper(PrimitiveType type,
                                   uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount);

    virtual void drawNonIndexedHelper(PrimitiveType type,
                                      uint32_t vertexCount,
                                      uint32_t numVertices);

    virtual void flushScissor(const GrIRect* rect);

    void eraseStencil(uint32_t value, uint32_t mask);
    virtual void eraseStencilClip();

    // flushes state that is common to fixed and programmable GL
    // dither
    // line smoothing
    // blend func
    // texture binding
    // sampler state (filtering, tiling)
    // FBO binding
    // line width
    void flushGLStateCommon(PrimitiveType type);

    // set when this class changes the rendertarget.
    // Subclass should notice at flush time, take appropriate action,
    // and set false.
    bool fRenderTargetChanged;

    // set by eraseColor or eraseStencil. Picked up in in flushStencil.
    bool fWriteMaskChanged;

    // last scissor / viewport scissor state seen by the GL.
    BoundsState fHWBounds;

private:
    GrGLExts fExts;

    GrGLRenderTarget* fDefaultRenderTarget;

    void resetContextHelper();

    // notify callbacks to update state tracking when related
    // objects are bound to GL or deleted outside of the class
    void notifyVertexBufferBind(const GrGLVertexBuffer* buffer);
    void notifyVertexBufferDelete(const GrGLVertexBuffer* buffer);
    void notifyIndexBufferBind(const GrGLIndexBuffer* buffer);
    void notifyIndexBufferDelete(const GrGLIndexBuffer* buffer);
    void notifyTextureBind(GrGLTexture* texture);
    void notifyTextureDelete(GrGLTexture* texture);
    void notifyRenderTargetDelete(GrRenderTarget* renderTarget);
    void notifyTextureRemoveRenderTarget(GrGLTexture* texture);

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

    typedef GrGpu INHERITED;
};

bool has_gl_extension(const char* ext);
void gl_version(int* major, int* minor);

/**
 *  GrGL_RestoreResetRowLength() will reset GL_UNPACK_ROW_LENGTH to 0. We write
 *  this wrapper, since GL_UNPACK_ROW_LENGTH is not available on all GL versions
 */
#if GR_GL_DESKTOP
    static inline void GrGL_RestoreResetRowLength() {
        GR_GL(PixelStorei(GL_UNPACK_ROW_LENGTH, 0));
    }
#else
    #define GrGL_RestoreResetRowLength()
#endif

#if SK_TextGLType != GL_FIXED
    #define SK_GL_HAS_COLOR4UB
#endif

#endif


