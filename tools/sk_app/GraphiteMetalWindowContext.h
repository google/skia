/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteMetalWindowContext_DEFINED
#define GraphiteMetalWindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"

#include "tools/sk_app/WindowContext.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

class SkSurface;

namespace sk_app {

class GraphiteMetalWindowContext : public WindowContext {
public:
    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return fValid; }

    void swapBuffers() override;

    void setDisplayParams(const DisplayParams& params) override;

    void activate(bool isActive) override;

protected:
    GraphiteMetalWindowContext(const DisplayParams&);
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change. This will in turn call onInitializeContext().
    void initializeContext();
    virtual bool onInitializeContext() = 0;

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new Metal context. This will in turn call
    // onDestroyContext().
    void destroyContext();
    virtual void onDestroyContext() = 0;

    bool                        fValid;
    sk_cfp<id<MTLDevice>>       fDevice;
    sk_cfp<id<MTLCommandQueue>> fQueue;
    CAMetalLayer*               fMetalLayer;
    CFTypeRef                   fDrawableHandle;
};

}   // namespace sk_app

#endif
