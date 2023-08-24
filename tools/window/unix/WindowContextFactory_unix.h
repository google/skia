
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_unix_DEFINED
#define WindowContextFactory_unix_DEFINED

// webgpu_cpp.h and X.h don't get along. Include this first, before X11 defines None, Success etc.
#ifdef SK_DAWN
#include "webgpu/webgpu_cpp.h"
#endif
#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>

typedef Window XWindow;

namespace skwindow {

class WindowContext;
struct DisplayParams;

struct XlibWindowInfo {
    Display*     fDisplay;
    XWindow      fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    int          fWidth;
    int          fHeight;
};

#ifdef SK_VULKAN
std::unique_ptr<WindowContext> MakeVulkanForXlib(const XlibWindowInfo&, const DisplayParams&);
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteVulkanForXlib(const XlibWindowInfo&,
                                                         const DisplayParams&);
#endif
#endif

#ifdef SK_GL
std::unique_ptr<WindowContext> MakeGLForXlib(const XlibWindowInfo&, const DisplayParams&);
#endif

#if defined(SK_DAWN) && defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteDawnVulkanForXlib(const XlibWindowInfo&,
                                                             const DisplayParams&);
#endif

std::unique_ptr<WindowContext> MakeRasterForXlib(const XlibWindowInfo&, const DisplayParams&);

}  // namespace skwindow

#endif
