/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkOSWindow_Unix_DEFINED
#define SkOSWindow_Unix_DEFINED

#include "SkWindow.h"
#include <X11/Xlib.h>

class SkBitmap;
class SkEvent;
class SkTypeface;

struct SkUnixWindow {
  Display* fDisplay;
  Window fWin;
  size_t fOSWin;
  GC fGc;
};

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*);
    ~SkOSWindow();

    void* getHWND() const { return (void*)fUnixWindow.fWin; }
    void* getDisplay() const { return (void*)fUnixWindow.fDisplay; }
    void* getUnixWindow() const { return (void*)&fUnixWindow; }
    void setUnixWindow(Display*, Window, size_t, GC);
    bool attachGL();
    void detachGL();
    void presentGL();
    void updatePointer(int x, int y);
    void toggleZoomer();
    bool zoomIn();
    bool zoomOut();

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

    void    doPaint();
    // Latest position of the mouse.
    int fMouseX, fMouseY;
    int fScale;
    // Used by the text showing position and color values.
    SkTypeface* fTypeface;
    bool fShowZoomer;

    typedef SkWindow INHERITED;
};

#endif

