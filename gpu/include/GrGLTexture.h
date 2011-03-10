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


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "GrTexture.h"
#include "GrScalar.h"
#include "GrGLIRect.h"

class GrGpuGL;
class GrGLTexture;

/**
 * A ref counted tex id that deletes the texture in its destructor.
 */
class GrGLTexID : public GrRefCnt {
public:
    GrGLTexID(GLuint texID) : fTexID(texID) {}

    virtual ~GrGLTexID() {
        if (0 != fTexID) {
            GR_GL(DeleteTextures(1, &fTexID));
        }
    }

    void abandon() { fTexID = 0; }
    GLuint id() const { return fTexID; }

private:
    GLuint      fTexID;
};

class GrGLRenderTarget : public GrRenderTarget {
public:
    virtual ~GrGLRenderTarget();
    
    bool resolveable() const { return fRTFBOID != fTexFBOID; }
    bool needsResolve() const { return fNeedsResolve; }
    void setDirty(bool dirty) { fNeedsResolve = resolveable() && dirty; }
    
    GLuint renderFBOID() const { return fRTFBOID; }
    GLuint textureFBOID() const { return fTexFBOID; }

    void   abandon();

protected:

    struct GLRenderTargetIDs {
        GLuint      fRTFBOID;
        GLuint      fTexFBOID;
        GLuint      fStencilRenderbufferID;
        GLuint      fMSColorRenderbufferID;
        bool        fOwnIDs;
    };
    
    GrGLRenderTarget(const GLRenderTargetIDs& ids,
                     GrGLTexID* texID,
                     GLuint stencilBits,
                     const GrGLIRect& fViewport,
                     GrGLTexture* texture,
                     GrGpuGL* gl);
    
    void setViewport(const GrGLIRect& rect) { fViewport = rect; }
    const GrGLIRect& getViewport() const { return fViewport; }
private:
    GrGpuGL*    fGL;
    GLuint      fRTFBOID;
    GLuint      fTexFBOID;
    GLuint      fStencilRenderbufferID;
    GLuint      fMSColorRenderbufferID;
   
    // Should this object delete IDs when it is destroyed or does someone
    // else own them.
    bool        fOwnIDs;
    
    // If there separate Texture and RenderTarget FBO IDs then the rendertarget
    // must be resolved to the texture FBO before it is used as a texture.
    bool fNeedsResolve;
    
    // when we switch to this rendertarget we want to set the viewport to 
    // only render to to content area (as opposed to the whole allocation) and
    // we want the rendering to be at top left (GL has origin in bottom left) 
    GrGLIRect fViewport;

    // non-NULL if this RT was created by Gr with an associated GrGLTexture.
    GrGLTexID* fTexIDObj;

    friend class GrGpuGL;
    friend class GrGLTexture;
    
    typedef GrRenderTarget INHERITED;
};

class GrGLTexture : public GrTexture {
public:
    enum Orientation {
        kBottomUp_Orientation,
        kTopDown_Orientation,
    };
    
    struct TexParams {
        GLenum fFilter;
        GLenum fWrapS;
        GLenum fWrapT;
    };

protected:
    struct GLTextureDesc {
        uint32_t    fContentWidth;
        uint32_t    fContentHeight;
        uint32_t    fAllocWidth;
        uint32_t    fAllocHeight;
        PixelConfig fFormat;
        GLuint      fTextureID;
        GLenum      fUploadFormat;
        GLenum      fUploadByteCount;
        GLenum      fUploadType;
        GLuint      fStencilBits;
        Orientation fOrientation;
    };
    typedef GrGLRenderTarget::GLRenderTargetIDs GLRenderTargetIDs;
    GrGLTexture(const GLTextureDesc& textureDesc,
                const GLRenderTargetIDs& rtIDs,
                const TexParams& initialTexParams,
                GrGpuGL* gl);

public:
    virtual ~GrGLTexture();
    
    // overloads of GrTexture
    virtual void abandon();
    virtual GrRenderTarget* asRenderTarget();
    virtual void releaseRenderTarget();
    virtual void uploadTextureData(uint32_t x,
                                   uint32_t y,
                                   uint32_t width,
                                   uint32_t height,
                                   const void* srcData);
    virtual intptr_t getTextureHandle();

    const TexParams& getTexParams() const { return fTexParams; }
    void setTexParams(const TexParams& texParams) { fTexParams = texParams; }
    GLuint textureID() const { return fTexIDObj->id(); }

    GLenum uploadFormat() const { return fUploadFormat; }
    GLenum uploadByteCount() const { return fUploadByteCount; }
    GLenum uploadType() const { return fUploadType; }

    /**
     * Retrieves the texture width actually allocated in texels.
     *
     * @return the width in texels
     */
    int allocWidth() const { return fAllocWidth; }

    /**
     * Retrieves the texture height actually allocated in texels.
     *
     * @return the height in texels
     */
    int allocHeight() const { return fAllocHeight; }

    /**
     * @return width() / allocWidth()
     */
    GrScalar contentScaleX() const { return fScaleX; }

    /**
     * @return height() / allocHeight()
     */
    GrScalar contentScaleY() const { return fScaleY; }

    // Ganesh assumes texture coordinates have their origin
    // in the top-left corner of the image. OpenGL, however,
    // has the origin in the lower-left corner. For content that
    // is loaded by Ganesh we just push the content "upside down"
    // (by GL's understanding of the world ) in glTex*Image and the 
    // addressing just works out. However, content generated by GL 
    // (FBO or externally imported texture) will be updside down 
    // and it is up to the GrGpuGL derivative to handle y-mirroing.
    Orientation orientation() const { return fOrientation; }

private:
    TexParams           fTexParams;
    GrGLTexID*          fTexIDObj;
    GLenum              fUploadFormat;
    GLenum              fUploadByteCount;
    GLenum              fUploadType;
    int                 fAllocWidth;
    int                 fAllocHeight;
    // precomputed content / alloc ratios
    GrScalar            fScaleX;
    GrScalar            fScaleY;
    Orientation         fOrientation;
    GrGLRenderTarget*   fRenderTarget;
    GrGpuGL*            fGpuGL;

    static const GLenum gWrapMode2GLWrap[];

    friend class GrGpuGL;

    typedef GrTexture INHERITED;
};

#endif
