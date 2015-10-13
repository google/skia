//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayGLX.h: GLX implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"

namespace rx
{

class FunctionsGLX;

class DisplayGLX : public DisplayGL
{
  public:
    DisplayGLX();
    ~DisplayGLX() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    SurfaceImpl *createWindowSurface(const egl::Config *configuration,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::Config *configuration,
                                      const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferFromClientBuffer(const egl::Config *configuration,
                                               EGLClientBuffer shareHandle,
                                               const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPixmapSurface(const egl::Config *configuration,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override;

    egl::ConfigSet generateConfigs() const override;

    bool isDeviceLost() const override;
    bool testDeviceLost() override;
    egl::Error restoreLostDevice() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    egl::Error getDevice(DeviceImpl **device) override;

    std::string getVendorString() const override;

    // Synchronizes with the X server, if the display has been opened by ANGLE.
    // Calling this is required at the end of every functions that does buffered
    // X calls (not for glX calls) otherwise there might be race conditions
    // between the application's display and ANGLE's one.
    void syncXCommands() const;

  private:
    const FunctionsGL *getFunctionsGL() const override;

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    int getGLXFBConfigAttrib(glx::FBConfig config, int attrib) const;

    FunctionsGL *mFunctionsGL;

    //TODO(cwallez) yuck, change generateConfigs to be non-const or add a userdata member to egl::Config?
    mutable std::map<int, glx::FBConfig> configIdToGLXConfig;

    glx::FBConfig mContextConfig;
    glx::Context mContext;
    // A pbuffer the context is current on during ANGLE initialization
    glx::Pbuffer mDummyPbuffer;

    bool mUsesNewXDisplay;

    FunctionsGLX mGLX;
    egl::Display *mEGLDisplay;
};

}

#endif // LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_
