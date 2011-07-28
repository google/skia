
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_Mac_DEFINED
#define SkOSWindow_Mac_DEFINED

#include <Carbon/Carbon.h>
#include "SkWindow.h"

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* hwnd);

    void*   getHWND() const { return fHWND; }
    void*   getHVIEW() const { return fHVIEW; }
    void    updateSize();

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    static OSStatus EventHandler(EventHandlerCallRef inHandler,
                                 EventRef inEvent, void* userData);

    void   doPaint(void* ctx);


    bool attachGL();
    void detachGL();
    void presentGL();

protected:
    // overrides from SkEventSink
    virtual bool onEvent(const SkEvent& evt);
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);
    virtual void onSetTitle(const char[]);
    

private:
    void*   fHWND;
    void*   fHVIEW;
    void*   fAGLCtx;

    typedef SkWindow INHERITED;
};

#endif

