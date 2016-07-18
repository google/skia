
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RasterWindowContext_android.h"

#include "SkSurface.h"
#include "SkTypes.h"

#include "Window_android.h"

namespace sk_app {

RasterWindowContext* RasterWindowContext::Create(void* platformData, const DisplayParams& params) {
    RasterWindowContext* ctx = new RasterWindowContext_android(platformData, params);
    if (!ctx->isValid()) {
        delete ctx;
        ctx = nullptr;
    }
    return ctx;
}

RasterWindowContext_android::RasterWindowContext_android(
        void* platformData, const DisplayParams& params) {
    fDisplayParams = params;
    ContextPlatformData_android* androidPlatformData =
            reinterpret_cast<ContextPlatformData_android*>(platformData);
    fNativeWindow = androidPlatformData->fNativeWindow;
    fWidth = ANativeWindow_getWidth(fNativeWindow);
    fHeight = ANativeWindow_getHeight(fNativeWindow);
    this->setBuffersGeometry();
}

void RasterWindowContext_android::setBuffersGeometry() {
    int32_t format = 0;
    switch(fDisplayParams.fColorType) {
        case kRGBA_8888_SkColorType:
            format = WINDOW_FORMAT_RGBA_8888;
            break;
        case kRGB_565_SkColorType:
            format = WINDOW_FORMAT_RGB_565;
            break;
        default:
            SK_ABORT("Unsupported Android color type");
    }
    ANativeWindow_setBuffersGeometry(fNativeWindow, fWidth, fHeight, format);
}

void RasterWindowContext_android::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    this->setBuffersGeometry();
}

void RasterWindowContext_android::resize(uint32_t w, uint32_t h) {
    fWidth = w;
    fHeight = h;
    this->setBuffersGeometry();
}

sk_sp<SkSurface> RasterWindowContext_android::getBackbufferSurface() {
    if (nullptr == fBackbufferSurface) {
        ANativeWindow_lock(fNativeWindow, &fBuffer, &fBounds);
        const int bytePerPixel = fBuffer.format == WINDOW_FORMAT_RGB_565 ? 2 : 4;
        SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                             fDisplayParams.fColorType,
                                             kPremul_SkAlphaType,
                                             fDisplayParams.fColorSpace);
        fBackbufferSurface = SkSurface::MakeRasterDirect(
                info, fBuffer.bits, fBuffer.stride * bytePerPixel, nullptr);
    }
    return fBackbufferSurface;
}


void RasterWindowContext_android::swapBuffers() {
    ANativeWindow_unlockAndPost(fNativeWindow);
    fBackbufferSurface.reset(nullptr);
}

}   // namespace sk_app
