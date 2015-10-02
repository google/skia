/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_Unix_DEFINED
#define SkOSWindow_Unix_DEFINED

#include <GL/glx.h>
#include <X11/Xlib.h>

#include "SkWindow.h"

class SkEvent;

struct SkUnixWindow {
  Display* fDisplay;
  Window fWin;
  size_t fOSWin;
  GC fGc;
  GLXContext fGLContext;
};

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*);
    ~SkOSWindow();

    void* getHWND() const { return (void*)fUnixWindow.fWin; }
    void* getDisplay() const { return (void*)fUnixWindow.fDisplay; }
    void* getUnixWindow() const { return (void*)&fUnixWindow; }
    void loop();

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
#if SK_ANGLE
        kANGLE_BackEndType,
#endif // SK_ANGLE
#if SK_COMMAND_BUFFER
        kCommandBuffer_BackEndType,
#endif // SK_COMMAND_BUFFER
    };

    bool attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo*);
    void detach();
    void present();

    int getMSAASampleCount() const { return fMSAASampleCount; }

    //static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    bool makeFullscreen();
    void setVsync(bool);
    void closeWindow();

protected:
    // Overridden from from SkWindow:
    void onSetTitle(const char title[]) override;

private:
    enum NextXEventResult {
        kContinue_NextXEventResult,
        kQuitRequest_NextXEventResult,
        kPaintRequest_NextXEventResult
    };

    NextXEventResult nextXEvent();
    void doPaint();
    void mapWindowAndWait();

    // Forcefully closes the window.  If a graceful shutdown is desired then call the public
    // closeWindow method
    void internalCloseWindow();
    void initWindow(int newMSAASampleCount, AttachmentInfo* info);

    SkUnixWindow fUnixWindow;

    // Needed for GL
    XVisualInfo* fVi;
    // we recreate the underlying xwindow if this changes
    int fMSAASampleCount;

    typedef SkWindow INHERITED;
};

#endif
