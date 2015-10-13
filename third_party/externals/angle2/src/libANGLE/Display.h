//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Display.h: Defines the egl::Display class, representing the abstract
// display on which graphics are drawn. Implements EGLDisplay.
// [EGL 1.4] section 2.1.2 page 3.

#ifndef LIBANGLE_DISPLAY_H_
#define LIBANGLE_DISPLAY_H_

#include <set>
#include <vector>

#include "libANGLE/Error.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Config.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/Renderer.h"

namespace gl
{
class Context;
}

namespace rx
{
class DisplayImpl;
}

namespace egl
{
class Device;
class Image;
class Surface;

class Display final : angle::NonCopyable
{
  public:
    ~Display();

    Error initialize();
    void terminate();

    static egl::Display *getDisplay(EGLNativeDisplayType displayId, const AttributeMap &attribMap);

    static const ClientExtensions &getClientExtensions();
    static const std::string &getClientExtensionString();

    std::vector<const Config*> getConfigs(const egl::AttributeMap &attribs) const;
    bool getConfigAttrib(const Config *configuration, EGLint attribute, EGLint *value);

    Error createWindowSurface(const Config *configuration, EGLNativeWindowType window, const AttributeMap &attribs,
                              Surface **outSurface);
    Error createPbufferSurface(const Config *configuration, const AttributeMap &attribs, Surface **outSurface);
    Error createPbufferFromClientBuffer(const Config *configuration, EGLClientBuffer shareHandle, const AttributeMap &attribs,
                                        Surface **outSurface);
    Error createPixmapSurface(const Config *configuration, NativePixmapType nativePixmap, const AttributeMap &attribs,
                              Surface **outSurface);

    Error createImage(gl::Context *context,
                      EGLenum target,
                      EGLClientBuffer buffer,
                      const AttributeMap &attribs,
                      Image **outImage);

    Error createContext(const Config *configuration, gl::Context *shareContext, const AttributeMap &attribs,
                        gl::Context **outContext);

    Error makeCurrent(egl::Surface *drawSurface, egl::Surface *readSurface, gl::Context *context);

    void destroySurface(egl::Surface *surface);
    void destroyImage(egl::Image *image);
    void destroyContext(gl::Context *context);

    bool isInitialized() const;
    bool isValidConfig(const Config *config) const;
    bool isValidContext(gl::Context *context) const;
    bool isValidSurface(egl::Surface *surface) const;
    bool isValidImage(const Image *image) const;
    bool isValidNativeWindow(EGLNativeWindowType window) const;

    static bool isValidDisplay(const egl::Display *display);
    static bool isValidNativeDisplay(EGLNativeDisplayType display);
    static bool hasExistingWindowSurface(EGLNativeWindowType window);

    bool isDeviceLost() const;
    bool testDeviceLost();
    void notifyDeviceLost();

    const Caps &getCaps() const;

    const DisplayExtensions &getExtensions() const;
    const std::string &getExtensionString() const;
    const std::string &getVendorString() const;

    const AttributeMap &getAttributeMap() const { return mAttributeMap; }
    EGLNativeDisplayType getNativeDisplayId() const { return mDisplayId; }

    rx::DisplayImpl *getImplementation() { return mImplementation; }
    Device *getDevice() const;

  private:
    Display(EGLNativeDisplayType displayId);

    void setAttributes(rx::DisplayImpl *impl, const AttributeMap &attribMap);

    Error restoreLostDevice();

    void initDisplayExtensions();
    void initVendorString();

    rx::DisplayImpl *mImplementation;

    EGLNativeDisplayType mDisplayId;
    AttributeMap mAttributeMap;

    ConfigSet mConfigSet;

    typedef std::set<gl::Context*> ContextSet;
    ContextSet mContextSet;

    typedef std::set<Image *> ImageSet;
    ImageSet mImageSet;

    bool mInitialized;

    Caps mCaps;

    DisplayExtensions mDisplayExtensions;
    std::string mDisplayExtensionString;

    std::string mVendorString;

    Device *mDevice;
};

}

#endif   // LIBANGLE_DISPLAY_H_
