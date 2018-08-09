/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef NXTWindowContext_DEFINED
#define NXTWindowContext_DEFINED

#include "SkRefCnt.h"
#include "SkSurface.h"

#include "WindowContext.h"
#include "dawn/dawncpp.h"
#include "dawn/dawn_wsi.h"

class GrContext;

namespace sk_app {

class NXTWindowContext : public WindowContext {
public:
    NXTWindowContext(const DisplayParams&, dawn::TextureFormat swapChainFormat);
    ~NXTWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fBackendContext.get()); }

    void resize(int w, int h) override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    virtual sk_sp<GrNXTBackendContext> onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual void onSwapBuffers() = 0;
    void destroyContext();
    virtual dawnSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) = 0;

    sk_sp<GrNXTBackendContext>    fBackendContext;
    dawn::Texture                 fTexture;
    sk_sp<SkSurface>              fSurface;
    dawnSwapChainImplementation   fSwapChainImplementation;
    dawn::TextureFormat           fSwapChainFormat;
    dawn::SwapChain               fSwapChain;
    dawn::Device                  fDevice;
};

}   // namespace sk_app

#endif
