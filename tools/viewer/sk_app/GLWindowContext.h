
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_DEFINED
#define GLWindowContext_DEFINED


#include "gl/GrGLInterface.h"

#include "SkRefCnt.h"
#include "GrRenderTarget.h"
#include "SkSurface.h"

#include "WindowContext.h"

class GrContext;

namespace sk_app {

class GLWindowContext : public WindowContext {
public:
    // This is defined in the platform .cpp file
    static GLWindowContext* Create(void* platformData, const DisplayParams& params);

    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return SkToBool(fBackendContext.get()); }

    void resize(uint32_t w, uint32_t h) override;
    void swapBuffers() override;

    void setDisplayParams(const DisplayParams& params) override;

    GrBackendContext getBackendContext() override {
        return (GrBackendContext) fBackendContext.get();
    }

protected:
    GLWindowContext(void*, const DisplayParams&);
    void initializeContext(void*, const DisplayParams&);
    virtual void onInitializeContext(void*, const DisplayParams&) = 0;
    void destroyContext();
    virtual void onDestroyContext() = 0;
    virtual void onSwapBuffers() = 0;

    SkAutoTUnref<const GrGLInterface> fBackendContext;
    sk_sp<GrRenderTarget>             fRenderTarget;
    sk_sp<SkSurface>                  fSurface;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount, fStencilBits, and fColorBits!
    int                               fSampleCount;
    int                               fStencilBits;
    int                               fColorBits;
    int                               fActualColorBits;
};

}   // namespace sk_app

#endif
