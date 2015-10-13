//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.h: Implementation methods of egl::Display

#ifndef LIBANGLE_RENDERER_DISPLAYIMPL_H_
#define LIBANGLE_RENDERER_DISPLAYIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Config.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/Renderer.h"

#include <set>
#include <vector>

namespace egl
{
class AttributeMap;
class Display;
struct Config;
class Surface;
class ImageSibling;
}

namespace gl
{
class Context;
}

namespace rx
{
class SurfaceImpl;
class ImageImpl;
struct ConfigDesc;
class DeviceImpl;

class DisplayImpl : angle::NonCopyable
{
  public:
    DisplayImpl();
    virtual ~DisplayImpl();

    virtual egl::Error initialize(egl::Display *display) = 0;
    virtual void terminate() = 0;

    virtual SurfaceImpl *createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs) = 0;
    virtual SurfaceImpl *createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs) = 0;
    virtual SurfaceImpl *createPbufferFromClientBuffer(const egl::Config *configuration,
                                                       EGLClientBuffer shareHandle,
                                                       const egl::AttributeMap &attribs) = 0;
    virtual SurfaceImpl *createPixmapSurface(const egl::Config *configuration,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs) = 0;

    virtual ImageImpl *createImage(EGLenum target,
                                   egl::ImageSibling *buffer,
                                   const egl::AttributeMap &attribs) = 0;

    virtual egl::Error createContext(const egl::Config *config, const gl::Context *shareContext, const egl::AttributeMap &attribs,
                                     gl::Context **outContext) = 0;

    virtual egl::Error makeCurrent(egl::Surface *drawSurface, egl::Surface *readSurface, gl::Context *context) = 0;

    virtual egl::ConfigSet generateConfigs() const = 0;

    virtual bool isDeviceLost() const = 0;
    virtual bool testDeviceLost() = 0;
    virtual egl::Error restoreLostDevice() = 0;

    virtual bool isValidNativeWindow(EGLNativeWindowType window) const = 0;

    virtual std::string getVendorString() const = 0;

    virtual egl::Error getDevice(DeviceImpl **device) = 0;

    const egl::Caps &getCaps() const;

    typedef std::set<egl::Surface*> SurfaceSet;
    const SurfaceSet &getSurfaceSet() const { return mSurfaceSet; }
    SurfaceSet &getSurfaceSet() { return mSurfaceSet; }

    void destroySurface(egl::Surface *surface);

    const egl::DisplayExtensions &getExtensions() const;

  protected:
    // Place the surface set here so it can be accessible for handling
    // context loss events. (It is shared between the Display and Impl.)
    SurfaceSet mSurfaceSet;

  private:
    virtual void generateExtensions(egl::DisplayExtensions *outExtensions) const = 0;
    virtual void generateCaps(egl::Caps *outCaps) const = 0;

    mutable bool mExtensionsInitialized;
    mutable egl::DisplayExtensions mExtensions;

    mutable bool mCapsInitialized;
    mutable egl::Caps mCaps;
};

}

#endif // LIBANGLE_RENDERER_DISPLAYIMPL_H_
