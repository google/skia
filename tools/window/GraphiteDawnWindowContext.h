/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteDawnWindowContext_DEFINED
#define GraphiteDawnWindowContext_DEFINED

#include "tools/window/WindowContext.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"

namespace skwindow::internal {

class GraphiteDawnWindowContext : public WindowContext {
public:
    GraphiteDawnWindowContext(const DisplayParams&, wgpu::TextureFormat swapChainFormat);
    ~GraphiteDawnWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    bool isValid() override { return SkToBool(fDevice.Get()); }
    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    wgpu::Device createDevice(wgpu::BackendType type);
    wgpu::SwapChain createSwapChain();
    void destroyContext();

    virtual bool onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual GrSurfaceOrigin getRTOrigin() const { return kTopLeft_GrSurfaceOrigin; }

    void onSwapBuffers() override;

    wgpu::TextureFormat                     fSwapChainFormat;
    std::unique_ptr<dawn::native::Instance> fInstance;
    wgpu::Device                            fDevice;
    wgpu::Queue                             fQueue;
    wgpu::Surface                           fSurface;
    wgpu::SwapChain                         fSwapChain;
};

}   // namespace skwindow::internal

#endif
