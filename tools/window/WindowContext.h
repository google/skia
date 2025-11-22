/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "tools/window/DisplayParams.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrTypes.h"
#endif

#include <functional>

class SkSurface;

#if defined(SK_GANESH)
class GrDirectContext;
#endif

#if defined(SK_GRAPHITE)
namespace skgpu::graphite {
class Context;
class Recorder;
}
#endif

namespace skwindow {

class WindowContext {
public:
    WindowContext(std::unique_ptr<const DisplayParams>);

    virtual ~WindowContext();

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    void swapBuffers();

    virtual bool isValid() = 0;

    virtual void resize(int w, int h) = 0;

    virtual void activate(bool isActive) {}

    const DisplayParams* getDisplayParams() { return fDisplayParams.get(); }
    virtual void setDisplayParams(std::unique_ptr<const DisplayParams>) = 0;

#if defined(SK_GANESH)
    GrDirectContext* directContext() const { return fContext.get(); }
#endif
#if defined(SK_GRAPHITE)
    skgpu::graphite::Context* graphiteContext() const { return fGraphiteContext.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() const { return fGraphiteRecorder.get(); }
#endif

    using GpuTimerCallback = std::function<void(uint64_t ns)>;
    void submitToGpu(GpuTimerCallback = {});
    bool supportsGpuTimer() const;

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkISize dimensions() const { return {fWidth, fHeight}; }
    int sampleCount() const { return fSampleCount; }
    int stencilBits() const { return fStencilBits; }

protected:
    virtual bool isGpuContext() { return true;  }

    virtual void onSwapBuffers() = 0;

#if defined(SK_GANESH)
    sk_sp<GrDirectContext> fContext;
#endif
#if defined(SK_GRAPHITE)
    std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
    std::unique_ptr<skgpu::graphite::Recorder> fGraphiteRecorder;
#endif

    int fWidth;
    int fHeight;
    std::unique_ptr<const DisplayParams> fDisplayParams;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount and fStencilBits!
    int               fSampleCount = 1;
    int               fStencilBits = 0;
};

}  // namespace skwindow

#endif
