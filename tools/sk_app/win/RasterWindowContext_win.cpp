/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "src/core/SkAutoMalloc.h"
#include "tools/sk_app/RasterWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

#include <Windows.h>

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;

namespace {

class RasterWindowContext_win : public RasterWindowContext {
public:
    RasterWindowContext_win(HWND, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fWnd); }
    void resize(int w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

protected:
    SkAutoMalloc fSurfaceMemory;
    sk_sp<SkSurface> fBackbufferSurface;
    HWND fWnd;

private:
    typedef RasterWindowContext INHERITED;
};

RasterWindowContext_win::RasterWindowContext_win(HWND wnd, const DisplayParams& params)
    : INHERITED(params)
    , fWnd(wnd) {
    RECT rect;
    GetClientRect(wnd, &rect);
    this->resize(rect.right - rect.left, rect.bottom - rect.top);
}

void RasterWindowContext_win::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    RECT rect;
    GetClientRect(fWnd, &rect);
    this->resize(rect.right - rect.left, rect.bottom - rect.top);
}

void RasterWindowContext_win::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    fBackbufferSurface.reset();
    const size_t bmpSize = sizeof(BITMAPINFOHEADER) + w * h * sizeof(uint32_t);
    fSurfaceMemory.reset(bmpSize);
    BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(fSurfaceMemory.get());
    ZeroMemory(bmpInfo, sizeof(BITMAPINFO));
    bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo->bmiHeader.biWidth = w;
    bmpInfo->bmiHeader.biHeight = -h; // negative means top-down bitmap. Skia draws top-down.
    bmpInfo->bmiHeader.biPlanes = 1;
    bmpInfo->bmiHeader.biBitCount = 32;
    bmpInfo->bmiHeader.biCompression = BI_RGB;
    void* pixels = bmpInfo->bmiColors;

    SkImageInfo info = SkImageInfo::Make(w, h, fDisplayParams.fColorType, kPremul_SkAlphaType,
                                         fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRasterDirect(info, pixels, sizeof(uint32_t) * w);
}

sk_sp<SkSurface> RasterWindowContext_win::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContext_win::swapBuffers() {
    BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(fSurfaceMemory.get());
    HDC dc = GetDC(fWnd);
    StretchDIBits(dc, 0, 0, fWidth, fHeight, 0, 0, fWidth, fHeight, bmpInfo->bmiColors, bmpInfo,
                  DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(fWnd, dc);
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForWin(HWND wnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_win(wnd, params));
    if (!ctx->isValid()) {
        ctx = nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
