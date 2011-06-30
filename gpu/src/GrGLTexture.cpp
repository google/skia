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


#include "GrGLTexture.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

GrGLRenderTarget::GrGLRenderTarget(GrGpuGL* gpu,
                                   const GLRenderTargetIDs& ids,
                                   GrGLTexID* texID,
                                   GrGLuint stencilBits,
                                   bool isMultisampled,
                                   const GrGLIRect& viewport,
                                   GrGLTexture* texture)
    : INHERITED(gpu, texture, viewport.fWidth, 
                viewport.fHeight, stencilBits, isMultisampled) {
    fRTFBOID                = ids.fRTFBOID;
    fTexFBOID               = ids.fTexFBOID;
    fStencilRenderbufferID  = ids.fStencilRenderbufferID;
    fMSColorRenderbufferID  = ids.fMSColorRenderbufferID;
    fViewport               = viewport;
    fOwnIDs                 = ids.fOwnIDs;
    fTexIDObj               = texID;
    GrSafeRef(fTexIDObj);
}

void GrGLRenderTarget::onRelease() {
    GPUGL->notifyRenderTargetDelete(this);
    if (fOwnIDs) {
        if (fTexFBOID) {
            GR_GL(DeleteFramebuffers(1, &fTexFBOID));
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            GR_GL(DeleteFramebuffers(1, &fRTFBOID));
        }
        if (fStencilRenderbufferID) {
            GR_GL(DeleteRenderbuffers(1, &fStencilRenderbufferID));
        }
        if (fMSColorRenderbufferID) {
            GR_GL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fStencilRenderbufferID  = 0;
    fMSColorRenderbufferID  = 0;
    GrSafeUnref(fTexIDObj);
    fTexIDObj = NULL;
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fStencilRenderbufferID  = 0;
    fMSColorRenderbufferID  = 0;
    if (NULL != fTexIDObj) {
        fTexIDObj->abandon();
        fTexIDObj = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////////

const GrGLenum* GrGLTexture::WrapMode2GLWrap() {
    static const GrGLenum mirrorRepeatModes[] = {
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_REPEAT,
        GR_GL_MIRRORED_REPEAT
    };

    static const GrGLenum repeatModes[] = {
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_REPEAT,
        GR_GL_REPEAT
    };

    if (GR_GL_SUPPORT_ES1 && !GR_GL_SUPPORT_ES2) {
        return repeatModes;  // GL_MIRRORED_REPEAT not supported.
    } else {
        return mirrorRepeatModes;
    }
};

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const GLTextureDesc& textureDesc,
                         const GLRenderTargetIDs& rtIDs,
                         const TexParams& initialTexParams)
    : INHERITED(gpu,
                textureDesc.fContentWidth,
                textureDesc.fContentHeight,
                textureDesc.fFormat) {

    fTexParams          = initialTexParams;
    fTexIDObj           = new GrGLTexID(textureDesc.fTextureID,
                                        textureDesc.fOwnsID);
    fUploadFormat       = textureDesc.fUploadFormat;
    fUploadByteCount    = textureDesc.fUploadByteCount;
    fUploadType         = textureDesc.fUploadType;
    fOrientation        = textureDesc.fOrientation;
    fAllocWidth         = textureDesc.fAllocWidth;
    fAllocHeight        = textureDesc.fAllocHeight;
    fScaleX             = GrIntToScalar(textureDesc.fContentWidth) /
                            textureDesc.fAllocWidth;
    fScaleY             = GrIntToScalar(textureDesc.fContentHeight) /
                            textureDesc.fAllocHeight;

    GrAssert(0 != textureDesc.fTextureID);

    if (rtIDs.fTexFBOID) {
        // we render to the top left
        GrGLIRect vp;
        vp.fLeft   = 0;
        vp.fWidth  = textureDesc.fContentWidth;
        vp.fHeight = textureDesc.fContentHeight;
        vp.fBottom = textureDesc.fAllocHeight - textureDesc.fContentHeight;

        fRenderTarget = new GrGLRenderTarget(gpu, rtIDs, fTexIDObj,
                                             textureDesc.fStencilBits,
                                             rtIDs.fRTFBOID != rtIDs.fTexFBOID,
                                             vp, this);
    }
}

void GrGLTexture::onRelease() {
    INHERITED::onRelease();
    GPUGL->notifyTextureDelete(this);
    if (NULL != fTexIDObj) {
        fTexIDObj->unref();
        fTexIDObj = NULL;
    }
}

void GrGLTexture::onAbandon() {
    INHERITED::onAbandon();
    if (NULL != fTexIDObj) {
        fTexIDObj->abandon();
    }
}

void GrGLTexture::uploadTextureData(int x,
                                    int y,
                                    int width,
                                    int height,
                                    const void* srcData,
                                    size_t rowBytes) {

    GPUGL->setSpareTextureUnit();

    // glCompressedTexSubImage2D doesn't support any formats
    // (at least without extensions)
    GrAssert(fUploadFormat != GR_GL_PALETTE8_RGBA8);

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> trimStorage;

    /*
     *  check if our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL doesn't let us pass the rowBytes as
     *  a parameter to glTexImage2D
     */

    if (GR_GL_SUPPORT_DESKTOP) {
        if (srcData && rowBytes) {
            GR_GL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH,
                              rowBytes / fUploadByteCount));
        }
    } else {
        size_t trimRowBytes = width * fUploadByteCount;
        if (srcData && (trimRowBytes < rowBytes)) {
            // copy the data into our new storage, skipping the trailing bytes
            size_t trimSize = height * trimRowBytes;
            const char* src = (const char*)srcData;
            char* dst = (char*)trimStorage.realloc(trimSize);
            for (int y = 0; y < height; y++) {
                memcpy(dst, src, trimRowBytes);
                src += rowBytes;
                dst += trimRowBytes;
            }
            // now point srcData to our trimmed version
            srcData = trimStorage.get();
        }
    }

    // If we need to update textures that are created upside down
    // then we have to modify this code to flip the srcData
    GrAssert(kTopDown_Orientation == fOrientation);
    GR_GL(BindTexture(GR_GL_TEXTURE_2D, fTexIDObj->id()));
    GR_GL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, fUploadByteCount));
    GR_GL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, x, y, width, height,
                        fUploadFormat, fUploadType, srcData));

    if (GR_GL_SUPPORT_DESKTOP) {
        if (srcData && rowBytes) {
            GR_GL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
    }
}

intptr_t GrGLTexture::getTextureHandle() {
    return fTexIDObj->id();
}

