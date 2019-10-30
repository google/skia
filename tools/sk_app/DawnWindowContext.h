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

#include "tools/sk_app/WindowContext.h"
#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn/dawn_wsi.h"

class GrContext;

namespace sk_app {

class DawnWindowContext : public WindowContext {
public:
    DawnWindowContext(const DisplayParams&, dawn::TextureFormat swapChainFormat);
    ~DawnWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fDevice.Get()); }

    void resize(int w, int h) override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    dawn::Device createDevice(dawn_native::BackendType type);
    virtual dawn::Device onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual void onSwapBuffers() = 0;
    virtual GrSurfaceOrigin getRTOrigin() const { return kTopLeft_GrSurfaceOrigin; }
    void destroyContext();
    virtual DawnSwapChainImplementation createSwapChainImplementation( int width, int height,
        const DisplayParams& params) = 0;

    sk_sp<SkSurface>              fSurface;
    DawnSwapChainImplementation   fSwapChainImplementation;
    dawn::TextureFormat           fSwapChainFormat;
    dawn::SwapChain               fSwapChain;
    dawn::Device                  fDevice;
    std::unique_ptr<dawn_native::Instance> fInstance;
};

}   // namespace sk_app

#endif
