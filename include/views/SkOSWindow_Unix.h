
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_Unix_DEFINED
#define SkOSWindow_Unix_DEFINED

#include "SkWindow.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

class SkBitmap;
class SkEvent;

struct SkUnixWindow {
  Display* fDisplay;
  Window fWin;
  size_t fOSWin;
  GC fGc;
  GLXContext fGLContext;
  bool fGLCreated;
};

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*);
    ~SkOSWindow();

    void* getHWND() const { return (void*)fUnixWindow.fWin; }
    void* getDisplay() const { return (void*)fUnixWindow.fDisplay; }
    void* getUnixWindow() const { return (void*)&fUnixWindow; }
    void loop();
    void post_linuxevent();
    bool attachGL();
    void detachGL();
    void presentGL();

    //static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    //static bool WndProc(SkUnixWindow* w,  XEvent &e);

protected:
    // overrides from SkWindow
    virtual bool onEvent(const SkEvent&);
    virtual void onHandleInval(const SkIRect&);
    virtual bool onHandleChar(SkUnichar);
    virtual bool onHandleKey(SkKey);
    virtual bool onHandleKeyUp(SkKey);
    virtual void onSetTitle(const char title[]);

private:
    SkUnixWindow  fUnixWindow;
    bool fGLAttached;

    // Needed for GL
    XVisualInfo* fVi;

    void    doPaint();
    void    mapWindowAndWait();

    typedef SkWindow INHERITED;
};

#endif

