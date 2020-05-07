/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

#include "include/core/SkSurface.h"

namespace sk_app {

class D3D12WindowContext : public WindowContext {
public:
    D3D12WindowContext(HWND hwnd, const DisplayParams& params);
    ~D3D12WindowContext() override;
    void initializeContext();
    void destroyContext();

    bool isValid() {
        return false;
    }

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    void resize(int width, int height) override { /* TODO */ }
    void setDisplayParams(const DisplayParams& params) override { /* TODO */ }
private:
    HWND                 fWindow;
};

D3D12WindowContext::D3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : WindowContext(params)
    , fWindow(hwnd) {
    // TODO
}

D3D12WindowContext::~D3D12WindowContext() {
    this->destroyContext();
}

void D3D12WindowContext::initializeContext() {
    // TODO
}

void D3D12WindowContext::destroyContext() {
    // TODO
}

sk_sp<SkSurface> D3D12WindowContext::getBackbufferSurface() {
    return nullptr;
}

void D3D12WindowContext::swapBuffers() {
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeD3D12ForWin(HWND hwnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new D3D12WindowContext(hwnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app
