/*
 * Copyright 2011 Skia
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_Android_DEFINED
#define SkOSWindow_Android_DEFINED

#include "SkWindow.h"

#include <EGL/egl.h>

struct SkAndroidWindow {
    EGLDisplay fDisplay;
    EGLSurface fSurface;
    EGLContext fContext;
};

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*);
    ~SkOSWindow();

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
    };

    bool attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo* info);
    void detach();
    void present();
    bool makeFullscreen() { return true; }
    void closeWindow();
    void setVsync(bool);
    bool destroyRequested() { return fDestroyRequested; }

    virtual void onPDFSaved(const char title[], const char desc[], const char path[]);

protected:
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    virtual void onSetTitle(const char title[]);

private:
    SkAndroidWindow fWindow;
    ANativeWindow* fNativeWindow;
    bool fDestroyRequested;

    typedef SkWindow INHERITED;
};

#endif
