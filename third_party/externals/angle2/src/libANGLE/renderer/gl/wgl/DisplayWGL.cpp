//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"

#include "common/debug.h"
#include "libANGLE/Config.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"
#include "libANGLE/renderer/gl/wgl/PbufferSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/WindowSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

#include <EGL/eglext.h>
#include <string>
#include <sstream>

namespace rx
{

class FunctionsGLWindows : public FunctionsGL
{
  public:
    FunctionsGLWindows(HMODULE openGLModule, PFNWGLGETPROCADDRESSPROC getProcAddressWGL)
        : mOpenGLModule(openGLModule),
          mGetProcAddressWGL(getProcAddressWGL)
    {
        ASSERT(mOpenGLModule);
        ASSERT(mGetProcAddressWGL);
    }

    virtual ~FunctionsGLWindows()
    {
    }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        void *proc = reinterpret_cast<void*>(mGetProcAddressWGL(function.c_str()));
        if (!proc)
        {
            proc = reinterpret_cast<void*>(GetProcAddress(mOpenGLModule, function.c_str()));
        }
        return proc;
    }

    HMODULE mOpenGLModule;
    PFNWGLGETPROCADDRESSPROC mGetProcAddressWGL;
};

DisplayWGL::DisplayWGL()
    : DisplayGL(),
      mOpenGLModule(nullptr),
      mFunctionsWGL(nullptr),
      mFunctionsGL(nullptr),
      mWindowClass(0),
      mWindow(nullptr),
      mDeviceContext(nullptr),
      mPixelFormat(0),
      mWGLContext(nullptr),
      mDisplay(nullptr)
{
}

DisplayWGL::~DisplayWGL()
{
}

egl::Error DisplayWGL::initialize(egl::Display *display)
{
    mDisplay = display;

    mOpenGLModule = LoadLibraryA("opengl32.dll");
    if (!mOpenGLModule)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to load OpenGL library.");
    }

    mFunctionsWGL = new FunctionsWGL();
    mFunctionsWGL->initialize(mOpenGLModule, nullptr);

    // WGL can't grab extensions until it creates a context because it needs to load the driver's DLLs first.
    // Create a dummy context to load the driver and determine which GL versions are available.

    // Work around compile error from not defining "UNICODE" while Chromium does
    const LPSTR idcArrow = MAKEINTRESOURCEA(32512);

    std::string className = FormatString("ANGLE DisplayWGL 0x%0.8p Intermediate Window Class", mDisplay);

    WNDCLASSA intermediateClassDesc = { 0 };
    intermediateClassDesc.style = CS_OWNDC;
    intermediateClassDesc.lpfnWndProc = DefWindowProc;
    intermediateClassDesc.cbClsExtra = 0;
    intermediateClassDesc.cbWndExtra = 0;
    intermediateClassDesc.hInstance = GetModuleHandle(nullptr);
    intermediateClassDesc.hIcon = nullptr;
    intermediateClassDesc.hCursor = LoadCursorA(nullptr, idcArrow);
    intermediateClassDesc.hbrBackground = 0;
    intermediateClassDesc.lpszMenuName = nullptr;
    intermediateClassDesc.lpszClassName = className.c_str();
    mWindowClass = RegisterClassA(&intermediateClassDesc);
    if (!mWindowClass)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to register intermediate OpenGL window class.");
    }

    HWND dummyWindow = CreateWindowExA(0,
                                       reinterpret_cast<const char *>(mWindowClass),
                                       "ANGLE Dummy Window",
                                       WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr);
    if (!dummyWindow)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create dummy OpenGL window.");
    }

    HDC dummyDeviceContext = GetDC(dummyWindow);
    if (!dummyDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get the device context of the dummy OpenGL window.");
    }

    const PIXELFORMATDESCRIPTOR pixelFormatDescriptor = wgl::GetDefaultPixelFormatDescriptor();

    int dummyPixelFormat = ChoosePixelFormat(dummyDeviceContext, &pixelFormatDescriptor);
    if (dummyPixelFormat == 0)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not find a compatible pixel format for the dummy OpenGL window.");
    }

    if (!SetPixelFormat(dummyDeviceContext, dummyPixelFormat, &pixelFormatDescriptor))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to set the pixel format on the intermediate OpenGL window.");
    }

    HGLRC dummyWGLContext = mFunctionsWGL->createContext(dummyDeviceContext);
    if (!dummyDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create a WGL context for the dummy OpenGL window.");
    }

    if (!mFunctionsWGL->makeCurrent(dummyDeviceContext, dummyWGLContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to make the dummy WGL context current.");
    }

    // Grab the GL version from this context and use it as the maximum version available.
    typedef const GLubyte* (GL_APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
    PFNGLGETSTRINGPROC getString = reinterpret_cast<PFNGLGETSTRINGPROC>(GetProcAddress(mOpenGLModule, "glGetString"));
    if (!getString)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get glGetString pointer.");
    }

    // Reinitialize the wgl functions to grab the extensions
    mFunctionsWGL->initialize(mOpenGLModule, dummyDeviceContext);

    // Destroy the dummy window and context
    mFunctionsWGL->makeCurrent(dummyDeviceContext, nullptr);
    mFunctionsWGL->deleteContext(dummyWGLContext);
    ReleaseDC(dummyWindow, dummyDeviceContext);
    DestroyWindow(dummyWindow);

    // Create the real intermediate context and windows
    mWindow = CreateWindowExA(0,
                              reinterpret_cast<const char *>(mWindowClass),
                              "ANGLE Intermediate Window",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr);
    if (!mWindow)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create intermediate OpenGL window.");
    }

    mDeviceContext = GetDC(mWindow);
    if (!mDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get the device context of the intermediate OpenGL window.");
    }

    if (mFunctionsWGL->choosePixelFormatARB)
    {
        std::vector<int> attribs = wgl::GetDefaultPixelFormatAttributes(false);

        UINT matchingFormats = 0;
        mFunctionsWGL->choosePixelFormatARB(mDeviceContext, &attribs[0], nullptr, 1u, &mPixelFormat,
                                            &matchingFormats);
    }

    if (mPixelFormat == 0)
    {
        mPixelFormat = ChoosePixelFormat(mDeviceContext, &pixelFormatDescriptor);
    }

    if (mPixelFormat == 0)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not find a compatible pixel format for the intermediate OpenGL window.");
    }

    if (!SetPixelFormat(mDeviceContext, mPixelFormat, &pixelFormatDescriptor))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to set the pixel format on the intermediate OpenGL window.");
    }

    if (mFunctionsWGL->createContextAttribsARB)
    {
        int flags = 0;
        // TODO: allow debug contexts
        // TODO: handle robustness

        int mask = 0;
        // Request core profile
        mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

        std::vector<int> contextCreationAttibutes;

        // Don't request a specific version unless the user wants one.  WGL will return the highest version
        // that the driver supports if no version is requested.
        const egl::AttributeMap &displayAttributes = display->getAttributeMap();
        EGLint requestedMajorVersion = displayAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
        EGLint requestedMinorVersion = displayAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);
        if (requestedMajorVersion != EGL_DONT_CARE && requestedMinorVersion != EGL_DONT_CARE)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
            contextCreationAttibutes.push_back(requestedMajorVersion);

            contextCreationAttibutes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
            contextCreationAttibutes.push_back(requestedMinorVersion);
        }

        // Set the flag attributes
        if (flags != 0)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_FLAGS_ARB);
            contextCreationAttibutes.push_back(flags);
        }

        // Set the mask attribute
        if (mask != 0)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
            contextCreationAttibutes.push_back(mask);
        }

        // Signal the end of the attributes
        contextCreationAttibutes.push_back(0);
        contextCreationAttibutes.push_back(0);

        mWGLContext = mFunctionsWGL->createContextAttribsARB(mDeviceContext, NULL, &contextCreationAttibutes[0]);
    }

    // If wglCreateContextAttribsARB is unavailable or failed, try the standard wglCreateContext
    if (!mWGLContext)
    {
        // Don't have control over GL versions
        mWGLContext = mFunctionsWGL->createContext(mDeviceContext);
    }

    if (!mWGLContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create a WGL context for the intermediate OpenGL window.");
    }

    if (!mFunctionsWGL->makeCurrent(mDeviceContext, mWGLContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to make the intermediate WGL context current.");
    }

    mFunctionsGL = new FunctionsGLWindows(mOpenGLModule, mFunctionsWGL->getProcAddress);
    mFunctionsGL->initialize();

    return DisplayGL::initialize(display);
}

void DisplayWGL::terminate()
{
    DisplayGL::terminate();

    mFunctionsWGL->makeCurrent(mDeviceContext, NULL);
    mFunctionsWGL->deleteContext(mWGLContext);
    mWGLContext = NULL;

    ReleaseDC(mWindow, mDeviceContext);
    mDeviceContext = NULL;

    DestroyWindow(mWindow);
    mWindow = NULL;

    UnregisterClassA(reinterpret_cast<const char*>(mWindowClass), NULL);
    mWindowClass = NULL;

    SafeDelete(mFunctionsWGL);
    SafeDelete(mFunctionsGL);

    FreeLibrary(mOpenGLModule);
    mOpenGLModule = nullptr;
}

SurfaceImpl *DisplayWGL::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    return new WindowSurfaceWGL(this->getRenderer(), window, mPixelFormat, mWGLContext,
                                mFunctionsWGL);
}

SurfaceImpl *DisplayWGL::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    EGLint width = attribs.get(EGL_WIDTH, 0);
    EGLint height = attribs.get(EGL_HEIGHT, 0);
    bool largest = (attribs.get(EGL_LARGEST_PBUFFER, EGL_FALSE) == EGL_TRUE);
    EGLenum textureFormat = attribs.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE);
    EGLenum textureTarget = attribs.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE);

    return new PbufferSurfaceWGL(this->getRenderer(), width, height, textureFormat, textureTarget,
                                 largest, mPixelFormat, mDeviceContext, mWGLContext, mFunctionsWGL);
}

SurfaceImpl *DisplayWGL::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                       EGLClientBuffer shareHandle,
                                                       const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayWGL::createPixmapSurface(const egl::Config *configuration,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::Error DisplayWGL::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::ConfigSet DisplayWGL::generateConfigs() const
{
    egl::ConfigSet configs;

    int minSwapInterval = 1;
    int maxSwapInterval = 1;
    if (mFunctionsWGL->swapIntervalEXT)
    {
        // No defined maximum swap interval in WGL_EXT_swap_control, use a reasonable number
        minSwapInterval = 0;
        maxSwapInterval = 8;
    }

    const gl::Version &maxVersion = getMaxSupportedESVersion();
    ASSERT(maxVersion >= gl::Version(2, 0));
    bool supportsES3 = maxVersion >= gl::Version(3, 0);

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    DescribePixelFormat(mDeviceContext, mPixelFormat, sizeof(pixelFormatDescriptor), &pixelFormatDescriptor);

    auto getAttrib = [this](int attrib)
    {
        return wgl::QueryWGLFormatAttrib(mDeviceContext, mPixelFormat, attrib, mFunctionsWGL);
    };

    egl::Config config;
    config.renderTargetFormat = GL_RGBA8; // TODO: use the bit counts to determine the format
    config.depthStencilFormat = GL_DEPTH24_STENCIL8; // TODO: use the bit counts to determine the format
    config.bufferSize = pixelFormatDescriptor.cColorBits;
    config.redSize = pixelFormatDescriptor.cRedBits;
    config.greenSize = pixelFormatDescriptor.cGreenBits;
    config.blueSize = pixelFormatDescriptor.cBlueBits;
    config.luminanceSize = 0;
    config.alphaSize = pixelFormatDescriptor.cAlphaBits;
    config.alphaMaskSize = 0;
    config.bindToTextureRGB   = (getAttrib(WGL_BIND_TO_TEXTURE_RGB_ARB) == TRUE);
    config.bindToTextureRGBA  = (getAttrib(WGL_BIND_TO_TEXTURE_RGBA_ARB) == TRUE);
    config.colorBufferType = EGL_RGB_BUFFER;
    config.configCaveat = EGL_NONE;
    config.conformant = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.depthSize = pixelFormatDescriptor.cDepthBits;
    config.level = 0;
    config.matchNativePixmap = EGL_NONE;
    config.maxPBufferWidth    = getAttrib(WGL_MAX_PBUFFER_WIDTH_ARB);
    config.maxPBufferHeight   = getAttrib(WGL_MAX_PBUFFER_HEIGHT_ARB);
    config.maxPBufferPixels   = getAttrib(WGL_MAX_PBUFFER_PIXELS_ARB);
    config.maxSwapInterval = maxSwapInterval;
    config.minSwapInterval = minSwapInterval;
    config.nativeRenderable = EGL_TRUE; // Direct rendering
    config.nativeVisualID = 0;
    config.nativeVisualType = EGL_NONE;
    config.renderableType = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.sampleBuffers = 0; // FIXME: enumerate multi-sampling
    config.samples = 0;
    config.stencilSize = pixelFormatDescriptor.cStencilBits;
    config.surfaceType =
        ((pixelFormatDescriptor.dwFlags & PFD_DRAW_TO_WINDOW) ? EGL_WINDOW_BIT : 0) |
        ((getAttrib(WGL_DRAW_TO_PBUFFER_ARB) == TRUE) ? EGL_PBUFFER_BIT : 0) |
        ((getAttrib(WGL_SWAP_METHOD_ARB) == WGL_SWAP_COPY_ARB) ? EGL_SWAP_BEHAVIOR_PRESERVED_BIT
                                                               : 0);
    config.transparentType = EGL_NONE;
    config.transparentRedValue = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue = 0;

    configs.add(config);

    return configs;
}

bool DisplayWGL::isDeviceLost() const
{
    //UNIMPLEMENTED();
    return false;
}

bool DisplayWGL::testDeviceLost()
{
    //UNIMPLEMENTED();
    return false;
}

egl::Error DisplayWGL::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayWGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (IsWindow(window) == TRUE);
}

std::string DisplayWGL::getVendorString() const
{
    //UNIMPLEMENTED();
    return "";
}

const FunctionsGL *DisplayWGL::getFunctionsGL() const
{
    return mFunctionsGL;
}

void DisplayWGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContext = true;
}

void DisplayWGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

}
