/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef XlibWindowInfo_DEFINED
#define XlibWindowInfo_DEFINED

#include <X11/X.h>      // Window (which is usually an unsigned long)
#include <X11/Xutil.h>  // XVisualInfo (which is an anonymous struct)

struct __GLXFBConfigRec;
using GLXFBConfig = __GLXFBConfigRec*;
using XWindow = Window;

namespace skwindow {

struct XlibWindowInfo {
    Display* fDisplay;
    XWindow fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    int fWidth;
    int fHeight;
};

}  // namespace skwindow

#endif
