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


#include "GrGLTexture.h"
#include "GrGpuGL.h"

GrGLRenderTarget::GrGLRenderTarget(const GLRenderTargetIDs& ids, 
                                   const GrIRect& viewport,
                                   GrGLTexture* texture,
                                   GrGpuGL* gl) : INHERITED(texture) {
    fGL                     = gl;
    fRTFBOID                = ids.fRTFBOID;
    fTexFBOID               = ids.fTexFBOID;
    fStencilRenderbufferID  = ids.fStencilRenderbufferID;
    fMSColorRenderbufferID  = ids.fMSColorRenderbufferID;
    fNeedsResolve           = false;
    fViewport               = viewport;
    fOwnIDs                 = ids.fOwnIDs;
    // viewport should be GL's viewport with top >= bottom
    GrAssert(viewport.height() <= 0);
}

GrGLRenderTarget::~GrGLRenderTarget() {
    fGL->notifyRenderTargetDelete(this);
    if (fOwnIDs) {
        if (fTexFBOID) {
            GR_GLEXT(fGL->extensions(), DeleteFramebuffers(1, &fTexFBOID));
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            GR_GLEXT(fGL->extensions(), DeleteFramebuffers(1, &fRTFBOID));
        }
        if (fStencilRenderbufferID) {
            GR_GLEXT(fGL->extensions(), DeleteRenderbuffers(1, &fStencilRenderbufferID));
        }
        if (fMSColorRenderbufferID) {
            GR_GLEXT(fGL->extensions(), DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
}

void GrGLRenderTarget::abandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fStencilRenderbufferID  = 0;
    fMSColorRenderbufferID  = 0;
}


////////////////////////////////////////////////////////////////////////////////

const GLenum GrGLTexture::gWrapMode2GLWrap[] = {
    GL_CLAMP_TO_EDGE,
    GL_REPEAT,
#ifdef GL_MIRRORED_REPEAT
    GL_MIRRORED_REPEAT
#else
    GL_REPEAT       // GL_MIRRORED_REPEAT not supported :(
#endif
};


GrGLTexture::GrGLTexture(const GLTextureDesc& textureDesc,
                         const GLRenderTargetIDs& rtIDs,
                         const TexParams& initialTexParams,
                         GrGpuGL* gl) :
        INHERITED(textureDesc.fContentWidth, 
                  textureDesc.fContentHeight, 
                  textureDesc.fAllocWidth, 
                  textureDesc.fAllocHeight,
                  textureDesc.fFormat),
        fTexParams(initialTexParams),
        fTextureID(textureDesc.fTextureID),
        fUploadFormat(textureDesc.fUploadFormat),
        fUploadByteCount(textureDesc.fUploadByteCount),
        fUploadType(textureDesc.fUploadType),
        fOrientation(textureDesc.fOrientation),
        fRenderTarget(NULL),
        fGpuGL(gl) {

    GrAssert(0 != textureDesc.fTextureID);

    if (rtIDs.fTexFBOID) {
        GrIRect vp;
        vp.fLeft   = 0;
        vp.fRight  = (int32_t) textureDesc.fContentWidth;
        // viewport for GL is top > bottom 
        vp.fTop    = (int32_t) textureDesc.fAllocHeight;
        vp.fBottom = (int32_t) textureDesc.fAllocHeight - 
                     (int32_t)textureDesc.fContentHeight;
        fRenderTarget = new GrGLRenderTarget(rtIDs, vp, this, gl);
    }
}

GrGLTexture::~GrGLTexture() {
    // make sure we haven't been abandoned
    if (fTextureID) {
        fGpuGL->notifyTextureDelete(this);
        GR_GL(DeleteTextures(1, &fTextureID));
    }
    delete fRenderTarget;
}

void GrGLTexture::abandon() {
    fTextureID = 0;
    if (NULL != fRenderTarget) {
        fRenderTarget->abandon();
    }
}

bool GrGLTexture::isRenderTarget() const {
    return NULL != fRenderTarget;
}

GrRenderTarget* GrGLTexture::asRenderTarget() {
    return (GrRenderTarget*)fRenderTarget;
}

void GrGLTexture::removeRenderTarget() {
    GrAssert(NULL != fRenderTarget);
    if (NULL != fRenderTarget) {
        // must do this notify before the delete
        fGpuGL->notifyTextureRemoveRenderTarget(this);
        delete fRenderTarget;
        fRenderTarget = NULL;        
    }
}

void GrGLTexture::uploadTextureData(uint32_t x,
                                    uint32_t y,
                                    uint32_t width,
                                    uint32_t height,
                                    const void* srcData) {
    
    fGpuGL->setSpareTextureUnit();

    // glCompressedTexSubImage2D doesn't support any formats
    // (at least without extensions)
    GrAssert(fUploadFormat != GR_PALETTE8_RGBA8);

    // If we need to update textures that are created upside down
    // then we have to modify this code to flip the srcData
    GrAssert(kTopDown_Orientation == fOrientation);
    GR_GL(BindTexture(GL_TEXTURE_2D, fTextureID));
    GR_GL(PixelStorei(GL_UNPACK_ALIGNMENT, fUploadByteCount));
    GR_GL(TexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, 
                        fUploadFormat, fUploadType, srcData));

}

intptr_t GrGLTexture::getTextureHandle() {
    return fTextureID;
}



