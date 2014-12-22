/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurface.h"
#include "GrSurfacePriv.h"

#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImageEncoder.h"
#include <stdio.h>

bool GrSurface::writePixels(int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer, size_t rowBytes,
                            uint32_t pixelOpsFlags) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return false;
    }
    return context->writeSurfacePixels(this, left, top, width, height, config, buffer, rowBytes,
                                       pixelOpsFlags);
}

bool GrSurface::readPixels(int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer, size_t rowBytes,
                           uint32_t pixelOpsFlags) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return false;
    }
    GrRenderTarget* target = this->asRenderTarget();
    if (target) {
        return context->readRenderTargetPixels(target, left, top, width, height, config, buffer,
                                               rowBytes, pixelOpsFlags);
    }
    return false;
}

SkImageInfo GrSurface::info() const {
    SkColorType colorType;
    SkColorProfileType profileType;
    if (!GrPixelConfig2ColorAndProfileType(this->config(), &colorType, &profileType)) {
        sk_throw();
    }
    return SkImageInfo::Make(this->width(), this->height(), colorType, kPremul_SkAlphaType,
                             profileType);
}

// TODO: This should probably be a non-member helper function. It might only be needed in
// debug or developer builds.
bool GrSurface::savePixels(const char* filename) {
    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(this->width(), this->height()))) {
        return false;
    }

    bool result = this->readPixels(0, 0, this->width(), this->height(), kSkia8888_GrPixelConfig,
                                   bm.getPixels());
    if (!result) {
        SkDebugf("------ failed to read pixels for %s\n", filename);
        return false;
    }

    // remove any previous version of this file
    remove(filename);

    if (!SkImageEncoder::EncodeFile(filename, bm, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return false;
    }

    return true;
}

void GrSurface::flushWrites() {
    if (!this->wasDestroyed()) {
        this->getContext()->flushSurfaceWrites(this);
    }
}

void GrSurface::prepareForExternalRead() {
    if (!this->wasDestroyed()) {
        this->getContext()->prepareSurfaceForExternalRead(this);
    }
}

bool GrSurface::hasPendingRead() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingRead()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingRead()) {
        return true;
    }
    return false;
}

bool GrSurface::hasPendingWrite() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingWrite()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingWrite()) {
        return true;
    }
    return false;
}

bool GrSurface::hasPendingIO() const {
    const GrTexture* thisTex = this->asTexture();
    if (thisTex && thisTex->internalHasPendingIO()) {
        return true;
    }
    const GrRenderTarget* thisRT = this->asRenderTarget();
    if (thisRT && thisRT->internalHasPendingIO()) {
        return true;
    }
    return false;
}
