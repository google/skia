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


#include "SkGrTexturePixelRef.h"

#include "GrTexture.h"

#include "SkRect.h"
#include "SkBitmap.h"

SkGrTexturePixelRef::SkGrTexturePixelRef(GrTexture* tex) {
    fTexture = tex;
    GrSafeRef(tex);
}

SkGrTexturePixelRef::~SkGrTexturePixelRef() {
    GrSafeUnref(fTexture);
}

bool SkGrTexturePixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL != fTexture && fTexture->isValid()) {
        int left, top, width, height;
        if (NULL != subset) {
            left = subset->fLeft;
            width = subset->width();
            top = subset->fTop;
            height = subset->height();
        } else {
            left = 0;
            width = fTexture->width();
            top = 0;
            height = fTexture->height();
        }
        dst->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        dst->allocPixels();
        SkAutoLockPixels al(*dst);
        void* buffer = dst->getPixels();
        return fTexture->readPixels(left, top, width, height,
                                    kRGBA_8888_GrPixelConfig,
                                    buffer);
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

SkGrRenderTargetPixelRef::SkGrRenderTargetPixelRef(GrRenderTarget* rt) {
    fRenderTarget = rt;
    GrSafeRef(fRenderTarget);
}

SkGrRenderTargetPixelRef::~SkGrRenderTargetPixelRef() {
    GrSafeUnref(fRenderTarget);
}

SkGpuTexture* SkGrRenderTargetPixelRef::getTexture() { 
    if (NULL != fRenderTarget) {
        return (SkGpuTexture*) fRenderTarget->asTexture();
    }
    return NULL;
}

bool SkGrRenderTargetPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL != fRenderTarget && fRenderTarget->isValid()) {
        int left, top, width, height;
        if (NULL != subset) {
            left = subset->fLeft;
            width = subset->width();
            top = subset->fTop;
            height = subset->height();
        } else {
            left = 0;
            width = fRenderTarget->width();
            top = 0;
            height = fRenderTarget->height();
        }
        dst->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        dst->allocPixels();
        SkAutoLockPixels al(*dst);
        void* buffer = dst->getPixels();
        return fRenderTarget->readPixels(left, top, width, height,
                                         kRGBA_8888_GrPixelConfig,
                                         buffer);
    } else {
        return false;
    }
}
