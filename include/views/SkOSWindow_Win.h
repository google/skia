
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_Win_DEFINED
#define SkOSWindow_Win_DEFINED

#include "SkWindow.h"

#if SK_ANGLE
#include "EGL/egl.h"
#endif

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* hwnd);
    virtual ~SkOSWindow();

    void*   getHWND() const { return fHWND; }
    void    setSize(int width, int height);
    void    updateSize();

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
#if SK_ANGLE
        kANGLE_BackEndType,
#endif
    };

    bool attach(SkBackEndTypes attachType, int msaaSampleCount);
    void detach();
    void present();

    bool wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static bool QuitOnDeactivate(HWND hWnd);

    enum {
        SK_WM_SkEvent = WM_APP + 1000,
        SK_WM_SkTimerID = 0xFFFF    // just need a non-zero value
    };

protected:
    virtual bool quitOnDeactivate() { return true; }

    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);

    virtual void onSetTitle(const char title[]);

private:
    void*               fHWND;
    
    void                doPaint(void* ctx);

    void*               fHGLRC;
#if SK_ANGLE
    EGLDisplay          fDisplay;
    EGLContext          fContext;
    EGLSurface          fSurface;
#endif

    HMENU               fMBar;

    SkBackEndTypes      fAttached;

    bool attachGL(int msaaSampleCount);
    void detachGL();
    void presentGL();

#if SK_ANGLE
    bool attachANGLE(int msaaSampleCount);
    void detachANGLE();
    void presentANGLE();
#endif

    typedef SkWindow INHERITED; 
};

#endif
