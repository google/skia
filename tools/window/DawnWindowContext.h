/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef DawnWindowContext_DEFINED
#define DawnWindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "tools/window/WindowContext.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"

namespace skwindow::internal {

class DawnWindowContext : public WindowContext {
public:
    DawnWindowContext(const DisplayParams&, wgpu::TextureFormat swapChainFormat);
    ~DawnWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    bool isValid() override { return SkToBool(fDevice.Get()); }

    void resize(int w, int h) override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    wgpu::Device createDevice(wgpu::BackendType type);
    virtual wgpu::Device onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual GrSurfaceOrigin getRTOrigin() const { return kTopLeft_GrSurfaceOrigin; }
    void destroyContext();
    void onSwapBuffers() override;

    sk_sp<SkSurface>              fSurface;
    wgpu::TextureFormat           fSwapChainFormat;
    wgpu::Surface                 fDawnSurface;
    wgpu::SwapChain               fSwapChain;
    wgpu::Device                  fDevice;
    std::unique_ptr<dawn::native::Instance> fInstance;
};

}   // namespace skwindow::internal

#endif
