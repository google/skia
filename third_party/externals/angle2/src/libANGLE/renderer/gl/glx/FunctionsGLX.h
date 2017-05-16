//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsGLX.h: Defines the FunctionsGLX class to load functions and data from GLX

#ifndef LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/glx/platform_glx.h"

namespace rx
{

class FunctionsGLX
{
  public:
    FunctionsGLX();
    ~FunctionsGLX();

    // Load data from GLX, can be called multiple times
    bool initialize(Display *xDisplay, int screen, std::string *errorString);
    void terminate();

    bool hasExtension(const char *extension) const;
    int majorVersion;
    int minorVersion;

    Display *getDisplay() const;
    int getScreen() const;

    PFNGETPROCPROC getProc;

    // GLX 1.0
    void destroyContext(glx::Context context) const;
    Bool makeCurrent(glx::Drawable drawable, glx::Context context) const;
    void swapBuffers(glx::Drawable drawable) const;
    Bool queryExtension(int *errorBase, int *event) const;
    Bool queryVersion(int *major, int *minor) const;
    void waitX() const;
    void waitGL() const;

    // GLX 1.1
    const char *queryExtensionsString() const;

    // GLX 1.3
    glx::FBConfig *getFBConfigs(int *nElements) const;
    glx::FBConfig *chooseFBConfig(const int *attribList, int *nElements) const;
    int getFBConfigAttrib(glx::FBConfig config, int attribute, int *value) const;
    XVisualInfo *getVisualFromFBConfig(glx::FBConfig config) const;
    glx::Window createWindow(glx::FBConfig config, Window window, const int *attribList) const;
    void destroyWindow(glx::Window window) const;
    glx::Pbuffer createPbuffer(glx::FBConfig config, const int *attribList) const;
    void destroyPbuffer(glx::Pbuffer pbuffer) const;
    void queryDrawable(glx::Drawable drawable, int attribute, unsigned int *value) const;

    // GLX_ARB_create_context
    glx::Context createContextAttribsARB(glx::FBConfig config, glx::Context shareContext, Bool direct, const int *attribList) const;

    // GLX_EXT_swap_control
    void swapIntervalEXT(glx::Drawable drawable, int interval) const;

  private:
    // So as to isolate GLX from angle we do not include angleutils.h and cannot
    // use angle::NonCopyable so we replicated it here instead.
    FunctionsGLX(const FunctionsGLX&) = delete;
    void operator=(const FunctionsGLX&) = delete;

    struct GLXFunctionTable;

    static void *sLibHandle;
    Display *mXDisplay;
    int mXScreen;

    GLXFunctionTable *mFnPtrs;
    std::vector<std::string> mExtensions;
};

}

#endif // LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLX_H_
