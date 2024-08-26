/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_DEFINED
#define GLWindowContext_DEFINED


#include "include/gpu/ganesh/gl/GrGLInterface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "tools/window/WindowContext.h"

namespace skwindow::internal {

class GLWindowContext : public WindowContext {
public:
    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return SkToBool(fBackendContext.get()); }

    void resize(int w, int h) override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    GLWindowContext(const DisplayParams&);
    // This should be called by subclass constructor. It is also called when window/display
    // parameters change. This will in turn call onInitializeContext().
    void initializeContext();
    virtual sk_sp<const GrGLInterface> onInitializeContext() = 0;

    // This should be called by subclass destructor. It is also called when window/display
    // parameters change prior to initializing a new GL context. This will in turn call
    // onDestroyContext().
    void destroyContext();
    virtual void onDestroyContext() = 0;

    sk_sp<const GrGLInterface> fBackendContext;
    sk_sp<SkSurface>           fSurface;
};

}  // namespace skwindow::internal

#endif
