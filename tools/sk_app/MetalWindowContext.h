
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef MetalWindowContext_DEFINED
#define MetalWindowContext_DEFINED


//#include "mtl/GrMtlInterface.h"

#include "SkRefCnt.h"
#include "SkSurface.h"

#include "WindowContext.h"

#import "Metal/Metal.h"

class GrContext;

namespace sk_app {

class MetalWindowContext : public WindowContext {
public:
    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return fValid; }//SkToBool(fBackendContext.get()); }

    void resize(int w, int h) override;
    void swapBuffers() override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    MetalWindowContext(const DisplayParams&);
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change. This will in turn call onInitializeContext().
    void initializeContext();
    virtual /*sk_sp<const GrGLInterface>*/ bool onInitializeContext() = 0;

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new GL context. This will in turn call
    // onDestroyContext().
    void destroyContext();
    virtual void onDestroyContext() = 0;

    virtual void onSwapBuffers() = 0;

//    sk_sp<const GrMTLInterface> fBackendContext;
    bool                        fValid;
    id<MTLDevice>               fDevice;
    id<MTLCommandQueue>         fQueue;
    sk_sp<SkSurface>            fSurface;
};

}   // namespace sk_app

#endif
