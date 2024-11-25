/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteDawnWindowContext_DEFINED
#define GraphiteDawnWindowContext_DEFINED

#include "tools/window/WindowContext.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE
#include "dawn/native/DawnNative.h"

namespace skwindow::internal {

class GraphiteDawnWindowContext : public WindowContext {
public:
    GraphiteDawnWindowContext(const DisplayParams&, wgpu::TextureFormat surfaceFormat);
    ~GraphiteDawnWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    bool isValid() override { return SkToBool(fDevice.Get()); }
    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    wgpu::Device createDevice(wgpu::BackendType type);
    void configureSurface();
    void destroyContext();

    virtual bool onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual GrSurfaceOrigin getRTOrigin() const { return kTopLeft_GrSurfaceOrigin; }

    void onSwapBuffers() override;

    wgpu::TextureFormat                     fSurfaceFormat;
    std::unique_ptr<dawn::native::Instance> fInstance;
    wgpu::Device                            fDevice;
    wgpu::Queue                             fQueue;
    wgpu::Surface                           fSurface;
};

}   // namespace skwindow::internal

#endif
