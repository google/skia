/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "DisplayParams.h"
#include "GrTypes.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class GrContext;
class SkSurface;
class GrRenderTarget;

namespace sk_app {

class WindowContext {
public:
    WindowContext() : fContext(nullptr)
                    , fSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType) {}

    virtual ~WindowContext() {}

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    virtual void swapBuffers() = 0;

    virtual bool isValid() = 0;

    virtual void resize(uint32_t w, uint32_t h) = 0;

    const DisplayParams& getDisplayParams() { return fDisplayParams; }
    virtual void setDisplayParams(const DisplayParams& params) = 0;

    SkSurfaceProps getSurfaceProps() const { return fSurfaceProps; }
    void setSurfaceProps(const SkSurfaceProps& props) {
        fSurfaceProps = props;
    }

    virtual GrBackendContext getBackendContext() = 0;
    GrContext* getGrContext() const { return fContext; }

    sk_sp<SkSurface> createOffscreenSurface(bool sRGB);
    sk_sp<SkSurface> createRenderSurface(sk_sp<GrRenderTarget>, int colorBits);
    void presentRenderSurface(sk_sp<SkSurface> renderSurface, sk_sp<GrRenderTarget> rt,
                              int colorBits);

protected:
    virtual bool isGpuContext() { return true;  }

    GrContext*        fContext;

    int               fWidth;
    int               fHeight;
    DisplayParams     fDisplayParams;
    GrPixelConfig     fPixelConfig;
    SkSurfaceProps    fSurfaceProps;

private:
    sk_sp<SkSurface> createSurface(
            sk_sp<GrRenderTarget>, int colorBits, bool offscreen, bool forceSRGB);
};

}   // namespace sk_app

#endif
