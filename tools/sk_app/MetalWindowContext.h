
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef MetalWindowContext_DEFINED
#define MetalWindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

using sk_app::window_context_factory::MacWindowInfo;

namespace sk_app {

class MetalWindowContext : public WindowContext {
public:
    MetalWindowContext(const MacWindowInfo& info, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return fValid; }

    void resize(int w, int h) override;
    void swapBuffers() override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change.
    void initializeContext();

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new Metal context.
    void destroyContext();

    NSView*                     fMainView;
    bool                        fValid;
    id<MTLDevice>               fDevice;
    id<MTLCommandQueue>         fQueue;
    CAMetalLayer*               fMetalLayer;
    id<CAMetalDrawable>         fCurrentDrawable;
    dispatch_semaphore_t        fInFlightSemaphore;
};

}   // namespace sk_app

#endif
