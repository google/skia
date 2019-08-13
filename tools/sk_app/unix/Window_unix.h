/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_unix_DEFINED
#define Window_unix_DEFINED

#include "include/private/SkChecksum.h"
#include "src/core/SkTDynamicHash.h"
#include "tools/sk_app/Window.h"

#include <GL/glx.h>
#include <X11/Xlib.h>

typedef Window XWindow;

namespace sk_app {

class Window_unix : public Window {
public:
    Window_unix() {}
    ~Window_unix() override { this->closeWindow(); }

    bool initWindow(Display* display);

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    bool handleEvent(const XEvent& event);

    static const XWindow& GetKey(const Window_unix& w) {
        return w.fWindow;
    }

    static uint32_t Hash(const XWindow& w) {
        return SkChecksum::Mix(w);
    }

    static SkTDynamicHash<Window_unix, XWindow> gWindowMap;

    void setRequestedDisplayParams(const DisplayParams&, bool allowReattach) override;

private:
    void closeWindow();

    Display*     fDisplay = nullptr;
    XWindow      fWindow = 0;
    GC           fGC = nullptr;
    GLXFBConfig* fFBConfig = nullptr;
    XVisualInfo* fVisualInfo = nullptr;
    int          fMSAASampleCount = 1;

    Atom     fWmDeleteMessage;

    BackendType fBackend;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
