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
#include "nxt/nxtcpp.h"
#include "nxt/nxt_wsi.h"

class GrContext;

namespace sk_app {

class NXTWindowContext : public WindowContext {
public:
    NXTWindowContext(const DisplayParams&, nxt::TextureFormat swapChainFormat);
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
    virtual nxtSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) = 0;

    sk_sp<GrNXTBackendContext>    fBackendContext;
    nxt::Texture                  fTexture;
    sk_sp<SkSurface>              fSurface;
    nxtSwapChainImplementation    fSwapChainImplementation;
    nxt::TextureFormat            fSwapChainFormat;
    nxt::SwapChain                fSwapChain;
    nxt::Device                   fDevice;
};

}   // namespace sk_app

#endif
