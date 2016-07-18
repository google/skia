
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_Win_DEFINED
#define SkOSWindow_Win_DEFINED

#include "../private/SkTHash.h"
#include "SkWindow.h"
#include <functional>

#if SK_ANGLE
#include "EGL/egl.h"
#endif

class SkOSWindow : public SkWindow {
public:
    struct WindowInit {
        const TCHAR*    fClass;
        HINSTANCE       fInstance;
    };

    SkOSWindow(const void* winInit);
    virtual ~SkOSWindow();

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    enum SkBackEndTypes {
        kNone_BackEndType,
#if SK_SUPPORT_GPU
        kNativeGL_BackEndType,
#if SK_ANGLE
        kANGLE_BackEndType,
#endif // SK_ANGLE
#endif // SK_SUPPORT_GPU
    };

    bool attach(SkBackEndTypes attachType, int msaaSampleCount, bool deepColor, AttachmentInfo*);
    void release();
    void present();

    bool wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static bool QuitOnDeactivate(HWND hWnd);

    enum {
        SK_WM_SkEvent = WM_APP + 1000,
        SK_WM_SkTimerID = 0xFFFF    // just need a non-zero value
    };

    bool makeFullscreen();
    void setVsync(bool);
    void closeWindow();

    static SkOSWindow* GetOSWindowForHWND(void* hwnd) {
        SkOSWindow** win = gHwndToOSWindowMap.find(hwnd);
        if (!win) {
            return NULL;
        }
        return *win;
    }

    // Iterates f over all the SkOSWindows and their corresponding HWNDs.
    // The void* argument to f is a HWND.
    static void ForAllWindows(const std::function<void(void*, SkOSWindow**)>& f) {
        gHwndToOSWindowMap.foreach(f);
    }

protected:
    virtual bool quitOnDeactivate() { return true; }

    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);

    virtual void onSetTitle(const char title[]);

private:
    static SkTHashMap<void*, SkOSWindow*> gHwndToOSWindowMap;

    WindowInit          fWinInit;
    void*               fHWND;

    void                doPaint(void* ctx);

#if SK_SUPPORT_GPU
    void*               fHGLRC;
#if SK_ANGLE
    EGLDisplay                        fDisplay;
    EGLContext                        fContext;
    EGLSurface                        fSurface;
    EGLConfig                         fConfig;
    SkAutoTUnref<const GrGLInterface> fANGLEInterface;
#endif // SK_ANGLE
#endif // SK_SUPPORT_GPU

    bool                fFullscreen;
    struct SavedWindowState {
        bool fZoomed;
        LONG fStyle;
        LONG fExStyle;
        RECT fRect;
        LONG fScreenWidth;
        LONG fScreenHeight;
        LONG fScreenBits;
        void* fHWND;
    } fSavedWindowState;

    HMENU               fMBar;

    SkBackEndTypes      fAttached;

    void updateSize();
#if SK_SUPPORT_GPU
    bool attachGL(int msaaSampleCount, bool deepColor, AttachmentInfo* info);
    void detachGL();
    void presentGL();

#if SK_ANGLE
    bool attachANGLE(int msaaSampleCount, AttachmentInfo* info);
    void detachANGLE();
    void presentANGLE();
#endif // SK_ANGLE

#endif // SK_SUPPORT_GPU

    typedef SkWindow INHERITED;
};

#endif
