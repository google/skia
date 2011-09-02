
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLTexture.h"

#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

const GrGLenum* GrGLTexture::WrapMode2GLWrap(GrGLBinding binding) {
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

    if (kES1_GrGLBinding == binding) {
        return repeatModes;  // GL_MIRRORED_REPEAT not supported.
    } else {
        return mirrorRepeatModes;
    }
};

void GrGLTexture::init(GrGpuGL* gpu,
                       const Desc& textureDesc,
                       const GrGLRenderTarget::Desc* rtDesc,
                       const TexParams& initialTexParams) {

    GrAssert(0 != textureDesc.fTextureID);

    fTexParams          = initialTexParams;
    fTexIDObj           = new GrGLTexID(GPUGL->glInterface(),
                                        textureDesc.fTextureID,
                                        textureDesc.fOwnsID);
    fUploadFormat       = textureDesc.fUploadFormat;
    fUploadByteCount    = textureDesc.fUploadByteCount;
    fUploadType         = textureDesc.fUploadType;
    fOrientation        = textureDesc.fOrientation;
    fScaleX             = GrIntToScalar(textureDesc.fContentWidth) /
                            textureDesc.fAllocWidth;
    fScaleY             = GrIntToScalar(textureDesc.fContentHeight) /
                            textureDesc.fAllocHeight;

    if (NULL != rtDesc) {
        // we render to the top left
        GrGLIRect vp;
        vp.fLeft   = 0;
        vp.fWidth  = textureDesc.fContentWidth;
        vp.fHeight = textureDesc.fContentHeight;
        vp.fBottom = textureDesc.fAllocHeight - textureDesc.fContentHeight;

        fRenderTarget = new GrGLRenderTarget(gpu, *rtDesc, vp, fTexIDObj, this);
    }
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const Desc& textureDesc,
                         const TexParams& initialTexParams) 
    : INHERITED(gpu,
                textureDesc.fContentWidth,
                textureDesc.fContentHeight,
                textureDesc.fAllocWidth,
                textureDesc.fAllocHeight,
                textureDesc.fFormat) {
    this->init(gpu, textureDesc, NULL, initialTexParams);
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const Desc& textureDesc,
                         const GrGLRenderTarget::Desc& rtDesc,
                         const TexParams& initialTexParams)
    : INHERITED(gpu,
                textureDesc.fContentWidth,
                textureDesc.fContentHeight,
                textureDesc.fAllocWidth,
                textureDesc.fAllocHeight,
                textureDesc.fFormat) {
    this->init(gpu, textureDesc, &rtDesc, initialTexParams);
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

    // ES2 glCompressedTexSubImage2D doesn't support any formats
    // (at least without extensions)
    GrAssert(fUploadFormat != GR_GL_PALETTE8_RGBA8);

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    if (!rowBytes) {
        rowBytes = fUploadByteCount * width;
    }
    /*
     *  check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES doesn't let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;
    bool flipY = kBottomUp_Orientation == fOrientation;
    if (kDesktop_GrGLBinding == GPUGL->glBinding() && !flipY) {
        // can't use this for flipping, only non-neg values allowed. :(
        if (srcData && rowBytes) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH,
                              rowBytes / fUploadByteCount));
            restoreGLRowLength = true;
        }
    } else {
        size_t trimRowBytes = width * fUploadByteCount;
        if (srcData && (trimRowBytes < rowBytes || flipY)) {
            // copy the data into our new storage, skipping the trailing bytes
            size_t trimSize = height * trimRowBytes;
            const char* src = (const char*)srcData;
            if (flipY) {
                src += (height - 1) * rowBytes;
            }
            char* dst = (char*)tempStorage.reset(trimSize);
            for (int y = 0; y < height; y++) {
                memcpy(dst, src, trimRowBytes);
                if (flipY) {
                    src -= rowBytes;
                } else {
                    src += rowBytes;
                }
                dst += trimRowBytes;
            }
            // now point srcData to our copied version
            srcData = tempStorage.get();
        }
    }

    if (flipY) {
        y = this->height() - (y + height);
    }
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, fTexIDObj->id()));
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, fUploadByteCount));
    GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, x, y, width, height,
                          fUploadFormat, fUploadType, srcData));

    if (kDesktop_GrGLBinding == GPUGL->glBinding()) {
        if (restoreGLRowLength) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
    }
}

intptr_t GrGLTexture::getTextureHandle() const {
    return fTexIDObj->id();
}

