
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowContextFactory_android.h"
#include "../RasterWindowContext.h"
#include "SkSurface.h"
#include "SkTypes.h"

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;

namespace {
class RasterWindowContext_android : public RasterWindowContext {
public:
    RasterWindowContext_android(ANativeWindow*, const DisplayParams& params);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return SkToBool(fNativeWindow); }
    void resize(int w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

private:
    void setBuffersGeometry();
    sk_sp<SkSurface> fBackbufferSurface = nullptr;
    ANativeWindow* fNativeWindow = nullptr;
    ANativeWindow_Buffer fBuffer;
    ARect fBounds;

    typedef RasterWindowContext INHERITED;
};

RasterWindowContext_android::RasterWindowContext_android(ANativeWindow* window,
                                                         const DisplayParams& params)
    : INHERITED(params) {
    fNativeWindow = window;
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

void RasterWindowContext_android::resize(int w, int h) {
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
}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewRasterForAndroid(ANativeWindow* window, const DisplayParams& params) {
    WindowContext* ctx = new RasterWindowContext_android(window, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}
}   // namespace sk_app
