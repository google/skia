/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrTypes.h"
#include "tools/sk_app/DisplayParams.h"

class GrDirectContext;
class SkSurface;

namespace sk_app {

class WindowContext {
public:
    WindowContext(const DisplayParams&);

    virtual ~WindowContext();

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    virtual void swapBuffers() = 0;

    virtual bool isValid() = 0;

    virtual void resize(int w, int h) = 0;

    virtual void activate(bool isActive) {}

    const DisplayParams& getDisplayParams() { return fDisplayParams; }
    virtual void setDisplayParams(const DisplayParams& params) = 0;

    GrDirectContext* directContext() const { return fContext.get(); }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCount() const { return fSampleCount; }
    int stencilBits() const { return fStencilBits; }

protected:
    virtual bool isGpuContext() { return true;  }

    sk_sp<GrDirectContext> fContext;

    int               fWidth;
    int               fHeight;
    DisplayParams     fDisplayParams;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount and fStencilBits!
    int               fSampleCount;
    int               fStencilBits;
};

}   // namespace sk_app

#endif
