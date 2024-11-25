
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "tools/window/DisplayParams.h"
#include "tools/window/RasterWindowContext.h"
#include "tools/window/android/WindowContextFactory_android.h"

using skwindow::internal::RasterWindowContext;
using skwindow::DisplayParams;

namespace {
class RasterWindowContext_android : public RasterWindowContext {
public:
    RasterWindowContext_android(ANativeWindow*, std::unique_ptr<const DisplayParams> params);

    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return SkToBool(fNativeWindow); }
    void resize(int w, int h) override;
    void setDisplayParams(std::unique_ptr<const DisplayParams> params) override;

private:
    void setBuffersGeometry();
    void onSwapBuffers() override;

    sk_sp<SkSurface> fBackbufferSurface = nullptr;
    ANativeWindow* fNativeWindow = nullptr;
    ANativeWindow_Buffer fBuffer;
    ARect fBounds;
};

RasterWindowContext_android::RasterWindowContext_android(
        ANativeWindow* window, std::unique_ptr<const DisplayParams> params)
        : RasterWindowContext(std::move(params)) {
    fNativeWindow = window;
    fWidth = ANativeWindow_getWidth(fNativeWindow);
    fHeight = ANativeWindow_getHeight(fNativeWindow);
    this->setBuffersGeometry();
}

void RasterWindowContext_android::setBuffersGeometry() {
    int32_t format = 0;
    switch (fDisplayParams->colorType()) {
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

void RasterWindowContext_android::setDisplayParams(std::unique_ptr<const DisplayParams> params) {
    fDisplayParams = std::move(params);
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
        SkImageInfo info = SkImageInfo::Make(fWidth,
                                             fHeight,
                                             fDisplayParams->colorType(),
                                             kPremul_SkAlphaType,
                                             fDisplayParams->colorSpace());
        fBackbufferSurface =
                SkSurfaces::WrapPixels(info, fBuffer.bits, fBuffer.stride * bytePerPixel, nullptr);
    }
    return fBackbufferSurface;
}


void RasterWindowContext_android::onSwapBuffers() {
    ANativeWindow_unlockAndPost(fNativeWindow);
    fBackbufferSurface.reset(nullptr);
}
}  // anonymous namespace

namespace skwindow {
std::unique_ptr<WindowContext> MakeRasterForAndroid(ANativeWindow* window,
                                                    std::unique_ptr<const DisplayParams> params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_android(window, std::move(params)));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}   // namespace skwindow
