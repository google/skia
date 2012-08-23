
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_MacCocoa_DEFINED
#define SkOSWindow_MacCocoa_DEFINED

#include "SkWindow.h"

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* hwnd);
    ~SkOSWindow();
    void*   getHWND() const { return fHWND; }

    virtual bool onDispatchClick(int x, int y, Click::State state,
                                 void* owner);
    enum SkBackEndTypes {
        kNone_BackEndType,
#if SK_SUPPORT_GPU
        kNativeGL_BackEndType,
#endif
    };

    void    detach();
    bool    attach(SkBackEndTypes attachType, int msaaSampleCount);
    void    present();

protected:
    // overrides from SkEventSink
    virtual bool onEvent(const SkEvent& evt);
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);
    virtual void onUpdateMenu(const SkOSMenu*);
    virtual void onSetTitle(const char[]);

private:
    void*   fHWND;
    bool    fInvalEventIsPending;
    void*   fNotifier;
#if SK_SUPPORT_GPU
    void*   fGLContext;
#endif
    typedef SkWindow INHERITED;
};

#endif
