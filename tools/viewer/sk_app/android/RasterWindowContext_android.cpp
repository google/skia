
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
    int32_t format;
    switch(params.fColorType) {
        case kRGBA_8888_SkColorType:
            format = WINDOW_FORMAT_RGBA_8888;
            break;
        case kRGB_565_SkColorType:
            format = WINDOW_FORMAT_RGB_565;
            break;
        default:
            SkDEBUGFAIL("Unsupported Android color type");
    }
    ANativeWindow_setBuffersGeometry(fNativeWindow, fWidth, fHeight, format);
}

sk_sp<SkSurface> RasterWindowContext_android::getBackbufferSurface() {
    if (nullptr == fBackbufferSurface) {
        ANativeWindow_lock(fNativeWindow, &fBuffer, &fBounds);
        const int bytePerPixel = fBuffer.format == WINDOW_FORMAT_RGB_565 ? 2 : 4;
        SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                             fDisplayParams.fColorType,
                                             kOpaque_SkAlphaType,
                                             fDisplayParams.fProfileType);
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
