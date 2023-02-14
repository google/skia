/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGLWindowContext_DEFINED
#define SkGLWindowContext_DEFINED


#include "include/gpu/gl/GrGLInterface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "tools/window/SkWindowContext.h"

class SkGLWindowContext : public SkWindowContext {
public:
    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return SkToBool(fBackendContext.get()); }

    void resize(int w, int h) override;
    void swapBuffers() override;

    void setDisplayParams(const SkDisplayParams& params) override;

protected:
    SkGLWindowContext(const SkDisplayParams&);
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change. This will in turn call onInitializeContext().
    void initializeContext();
    virtual sk_sp<const GrGLInterface> onInitializeContext() = 0;

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new GL context. This will in turn call
    // onDestroyContext().
    void destroyContext();
    virtual void onDestroyContext() = 0;

    virtual void onSwapBuffers() = 0;

    sk_sp<const GrGLInterface> fBackendContext;
    sk_sp<SkSurface>           fSurface;
};

#endif
