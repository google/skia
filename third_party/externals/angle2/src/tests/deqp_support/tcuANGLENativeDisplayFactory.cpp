/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "tcuANGLENativeDisplayFactory.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "deClock.h"
#include "deMemory.h"
#include "deThread.h"
#include "egluDefs.hpp"
#include "eglwLibrary.hpp"
#include "OSPixmap.h"
#include "OSWindow.h"
#include "tcuTexture.hpp"

#if (DE_OS == DE_OS_WIN32)
    #define ANGLE_EGL_LIBRARY_NAME "libEGL.dll"
#elif (DE_OS == DE_OS_UNIX)
    #define ANGLE_EGL_LIBRARY_NAME "libEGL.so"
#elif (DE_OS == DE_OS_OSX)
    #define ANGLE_EGL_LIBRARY_NAME "libEGL.dylib"
#else
    #error "Unsupported platform"
#endif

namespace tcu
{
namespace
{

enum
{
    DEFAULT_SURFACE_WIDTH       = 400,
    DEFAULT_SURFACE_HEIGHT      = 300,
    WAIT_WINDOW_VISIBLE_MS      = 500   //!< Time to wait before issuing screenshot after changing window visibility (hack for DWM)
};

static const eglu::NativeDisplay::Capability DISPLAY_CAPABILITIES = eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM;
static const eglu::NativePixmap::Capability  BITMAP_CAPABILITIES  = eglu::NativePixmap::CAPABILITY_CREATE_SURFACE_LEGACY;
static const eglu::NativeWindow::Capability  WINDOW_CAPABILITIES  = (eglu::NativeWindow::Capability)
                                                                     (eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY    |
                                                                      eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE         |
                                                                      eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE          |
                                                                      eglu::NativeWindow::CAPABILITY_READ_SCREEN_PIXELS       |
                                                                      eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE         |
                                                                      eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY);

class ANGLENativeDisplay : public eglu::NativeDisplay
{
  public:
    ANGLENativeDisplay(const std::vector<eglw::EGLAttrib> &attribs);
    virtual ~ANGLENativeDisplay() {}

    void *getPlatformNative() override { return mDeviceContext; }
    const eglw::EGLAttrib *getPlatformAttributes() const override { return &mPlatformAttributes[0]; }
    const eglw::Library &getLibrary() const override { return mLibrary; }

    EGLNativeDisplayType getDeviceContext() { return mDeviceContext; }

  private:
    EGLNativeDisplayType mDeviceContext;
    eglw::DefaultLibrary mLibrary;
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};

class NativePixmapFactory : public eglu::NativePixmapFactory
{
  public:
    NativePixmapFactory();
    ~NativePixmapFactory() {}

    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay, int width, int height) const override;
    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay, eglw::EGLDisplay display, eglw::EGLConfig config, const eglw::EGLAttrib* attribList, int width, int height) const override;
};

class NativePixmap : public eglu::NativePixmap
{
  public:
    NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth);
    virtual ~NativePixmap();

    eglw::EGLNativePixmapType getLegacyNative() override;

  private:
    OSPixmap *mPixmap;
};

class NativeWindowFactory : public eglu::NativeWindowFactory
{
  public:
    NativeWindowFactory(EventState *eventState);
    ~NativeWindowFactory() override {}

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay, const eglu::WindowParams &params) const override;

  private:
    EventState *mEvents;
};

class NativeWindow : public eglu::NativeWindow
{
  public:
    NativeWindow(ANGLENativeDisplay *nativeDisplay, const eglu::WindowParams &params, EventState *eventState);
    ~NativeWindow() override;

    eglw::EGLNativeWindowType getLegacyNative() override;
    IVec2 getSurfaceSize() const override;
    IVec2 getScreenSize() const override { return getSurfaceSize(); }
    void processEvents() override;
    void setSurfaceSize(IVec2 size) override;
    void setVisibility(eglu::WindowParams::Visibility visibility) override;
    void readScreenPixels(tcu::TextureLevel* dst) const override;

  private:
    OSWindow *mWindow;
    EventState *mEvents;
};

// ANGLE NativeDisplay

ANGLENativeDisplay::ANGLENativeDisplay(const std::vector<EGLAttrib> &attribs)
    : eglu::NativeDisplay(DISPLAY_CAPABILITIES, EGL_PLATFORM_ANGLE_ANGLE, "EGL_EXT_platform_base"),
      mDeviceContext(EGL_DEFAULT_DISPLAY),
      mLibrary(ANGLE_EGL_LIBRARY_NAME),
      mPlatformAttributes(attribs)
{
}

// NativePixmap

NativePixmap::NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth)
    : eglu::NativePixmap(BITMAP_CAPABILITIES),
      mPixmap(CreateOSPixmap())
{
    if (!mPixmap)
    {
        throw ResourceError("Failed to create pixmap", DE_NULL, __FILE__, __LINE__);
    }

    if (!mPixmap->initialize(display, width, height, bitDepth))
    {
        throw ResourceError("Failed to initialize pixmap", DE_NULL, __FILE__, __LINE__);
    }
}

NativePixmap::~NativePixmap()
{
    delete mPixmap;
}

eglw::EGLNativePixmapType NativePixmap::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativePixmapType>(mPixmap->getNativePixmap());
}

// NativePixmapFactory

NativePixmapFactory::NativePixmapFactory()
    : eglu::NativePixmapFactory("bitmap", "ANGLE Bitmap", BITMAP_CAPABILITIES)
{
}

eglu::NativePixmap *NativePixmapFactory::createPixmap (eglu::NativeDisplay* nativeDisplay, eglw::EGLDisplay display, eglw::EGLConfig config, const eglw::EGLAttrib* attribList, int width, int height) const
{
    const eglw::Library&    egl         = nativeDisplay->getLibrary();
    int             redBits     = 0;
    int             greenBits   = 0;
    int             blueBits    = 0;
    int             alphaBits   = 0;
    int             bitSum      = 0;

    DE_ASSERT(display != EGL_NO_DISPLAY);

    egl.getConfigAttrib(display, config, EGL_RED_SIZE,      &redBits);
    egl.getConfigAttrib(display, config, EGL_GREEN_SIZE,    &greenBits);
    egl.getConfigAttrib(display, config, EGL_BLUE_SIZE,     &blueBits);
    egl.getConfigAttrib(display, config, EGL_ALPHA_SIZE,    &alphaBits);
    EGLU_CHECK_MSG(egl, "eglGetConfigAttrib()");

    bitSum = redBits+greenBits+blueBits+alphaBits;

    return new NativePixmap(dynamic_cast<ANGLENativeDisplay*>(nativeDisplay)->getDeviceContext(), width, height, bitSum);
}

eglu::NativePixmap *NativePixmapFactory::createPixmap(eglu::NativeDisplay* nativeDisplay, int width, int height) const
{
    const int defaultDepth = 32;
    return new NativePixmap(dynamic_cast<ANGLENativeDisplay*>(nativeDisplay)->getDeviceContext(), width, height, defaultDepth);
}

// NativeWindowFactory

NativeWindowFactory::NativeWindowFactory(EventState *eventState)
    : eglu::NativeWindowFactory("window", "ANGLE Window", WINDOW_CAPABILITIES),
      mEvents(eventState)
{
}

eglu::NativeWindow *NativeWindowFactory::createWindow(eglu::NativeDisplay* nativeDisplay, const eglu::WindowParams& params) const
{
    return new NativeWindow(dynamic_cast<ANGLENativeDisplay*>(nativeDisplay), params, mEvents);
}

// NativeWindow

NativeWindow::NativeWindow(ANGLENativeDisplay *nativeDisplay, const eglu::WindowParams& params, EventState *eventState)
    : eglu::NativeWindow(WINDOW_CAPABILITIES),
      mWindow(CreateOSWindow()),
      mEvents(eventState)
{
    bool initialized = mWindow->initialize("dEQP ANGLE Tests",
                                           params.width == eglu::WindowParams::SIZE_DONT_CARE ? DEFAULT_SURFACE_WIDTH : params.width,
                                           params.height == eglu::WindowParams::SIZE_DONT_CARE ? DEFAULT_SURFACE_HEIGHT : params.height);
    TCU_CHECK(initialized);

    if (params.visibility != eglu::WindowParams::VISIBILITY_DONT_CARE)
        setVisibility(params.visibility);
}

void NativeWindow::setVisibility(eglu::WindowParams::Visibility visibility)
{
    switch (visibility)
    {
      case eglu::WindowParams::VISIBILITY_HIDDEN:
        mWindow->setVisible(false);
        break;

      case eglu::WindowParams::VISIBILITY_VISIBLE:
      case eglu::WindowParams::VISIBILITY_FULLSCREEN:
        // \todo [2014-03-12 pyry] Implement FULLSCREEN, or at least SW_MAXIMIZE.
        mWindow->setVisible(true);
        break;

      default:
        DE_ASSERT(DE_FALSE);
    }
}

NativeWindow::~NativeWindow()
{
    delete mWindow;
}

eglw::EGLNativeWindowType NativeWindow::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativeWindowType>(mWindow->getNativeWindow());
}

IVec2 NativeWindow::getSurfaceSize() const
{
    return IVec2(mWindow->getWidth(), mWindow->getHeight());
}

void NativeWindow::processEvents()
{
    mWindow->messageLoop();

    // Look for a quit event to forward to the EventState
    Event event;
    while (mWindow->popEvent(&event))
    {
        if (event.Type == Event::EVENT_CLOSED)
        {
            mEvents->signalQuitEvent();
        }
    }
}

void NativeWindow::setSurfaceSize(IVec2 size)
{
    mWindow->resize(size.x(), size.y());
}

void NativeWindow::readScreenPixels(tcu::TextureLevel *dst) const
{
    dst->setStorage(TextureFormat(TextureFormat::BGRA, TextureFormat::UNORM_INT8), mWindow->getWidth(), mWindow->getHeight());
    bool success = mWindow->takeScreenshot(reinterpret_cast<uint8_t*>(dst->getAccess().getDataPtr()));
    DE_ASSERT(success);
    DE_UNREF(success);
}

} // anonymous

ANGLENativeDisplayFactory::ANGLENativeDisplayFactory(const std::string &name,
                                                     const std::string &description,
                                                     const std::vector<eglw::EGLAttrib> &platformAttributes,
                                                     EventState *eventState)
    : eglu::NativeDisplayFactory(name, description, DISPLAY_CAPABILITIES, EGL_PLATFORM_ANGLE_ANGLE, "EGL_EXT_platform_base"),
      mPlatformAttributes(platformAttributes)
{
    m_nativeWindowRegistry.registerFactory(new NativeWindowFactory(eventState));
    m_nativePixmapRegistry.registerFactory(new NativePixmapFactory());
}

ANGLENativeDisplayFactory::~ANGLENativeDisplayFactory()
{
}

eglu::NativeDisplay *ANGLENativeDisplayFactory::createDisplay(const eglw::EGLAttrib *attribList) const
{
    DE_UNREF(attribList);
    return new ANGLENativeDisplay(mPlatformAttributes);
}

} // tcu
