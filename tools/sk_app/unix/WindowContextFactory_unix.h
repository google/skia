
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_unix_DEFINED
#define WindowContextFactory_unix_DEFINED

#include <X11/Xlib.h>
#include <GL/glx.h>
typedef Window XWindow;

namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

struct XlibWindowInfo {
    Display*     fDisplay;
    XWindow      fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    int          fWidth;
    int          fHeight;
};

WindowContext* NewVulkanForXlib(const XlibWindowInfo&, const DisplayParams&);

WindowContext* NewGLForXlib(const XlibWindowInfo&, const DisplayParams&);

WindowContext* NewRasterForXlib(const XlibWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
