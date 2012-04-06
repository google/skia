
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOSWindow_iOS_DEFINED
#define SkOSWindow_iOS_DEFINED

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
        kNativeGL_BackEndType,
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
    typedef SkWindow INHERITED;
};

#endif

