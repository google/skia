
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

const GrGLenum* GrGLTexture::WrapMode2GLWrap() {
    static const GrGLenum repeatModes[] = {
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_REPEAT,
        GR_GL_MIRRORED_REPEAT
    };
    return repeatModes;
};

void GrGLTexture::init(GrGpuGL* gpu,
                       const Desc& textureDesc,
                       const GrGLRenderTarget::Desc* rtDesc) {

    GrAssert(0 != textureDesc.fTextureID);

    fTexParams.invalidate();
    fTexParamsTimestamp = GrGpu::kExpiredTimestamp;
    fTexIDObj           = new GrGLTexID(GPUGL->glInterface(),
                                        textureDesc.fTextureID,
                                        textureDesc.fOwnsID);
    fUploadFormat       = textureDesc.fUploadFormat;
    fUploadType         = textureDesc.fUploadType;
    fOrientation        = textureDesc.fOrientation;

    if (NULL != rtDesc) {
        // we render to the top left
        GrGLIRect vp;
        vp.fLeft   = 0;
        vp.fWidth  = textureDesc.fWidth;
        vp.fBottom = 0;
        vp.fHeight = textureDesc.fHeight;

        fRenderTarget = new GrGLRenderTarget(gpu, *rtDesc, vp, fTexIDObj, this);
    }
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const Desc& textureDesc) 
    : INHERITED(gpu,
                textureDesc.fWidth,
                textureDesc.fHeight,
                textureDesc.fConfig) {
    this->init(gpu, textureDesc, NULL);
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const Desc& textureDesc,
                         const GrGLRenderTarget::Desc& rtDesc)
    : INHERITED(gpu,
                textureDesc.fWidth,
                textureDesc.fHeight,
                textureDesc.fConfig) {
    this->init(gpu, textureDesc, &rtDesc);
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
    GrIRect bounds = GrIRect::MakeWH(this->width(), this->height());
    GrIRect subrect = GrIRect::MakeXYWH(x,y,width, height);
    if (!bounds.contains(subrect)) {
        return;
    }
    GPUGL->setSpareTextureUnit();

    // ES2 glCompressedTexSubImage2D doesn't support any formats
    // (at least without extensions)
    GrAssert(fUploadFormat != GR_GL_PALETTE8_RGBA8);

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    size_t bpp = GrBytesPerPixel(this->config());
    size_t trimRowBytes = width * bpp;
    if (!rowBytes) {
        rowBytes = trimRowBytes;
    }
    /*
     *  check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES may not let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;
    bool flipY = kBottomUp_Orientation == fOrientation;
    if (GPUGL->glCaps().fUnpackRowLengthSupport && !flipY) {
        // can't use this for flipping, only non-neg values allowed. :(
        if (srcData && rowBytes != trimRowBytes) {
            GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
            restoreGLRowLength = true;
        }
    } else {
        if (srcData && (trimRowBytes != rowBytes || flipY)) {
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
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, static_cast<GrGLint>(bpp)));
    GL_CALL(TexSubImage2D(GR_GL_TEXTURE_2D, 0, x, y, width, height,
                          fUploadFormat, fUploadType, srcData));

    if (restoreGLRowLength) {
        GrAssert(GPUGL->glCaps().fUnpackRowLengthSupport);
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
}

intptr_t GrGLTexture::getTextureHandle() const {
    return fTexIDObj->id();
}

