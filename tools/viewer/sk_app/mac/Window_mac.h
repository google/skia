/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_mac_DEFINED
#define Window_mac_DEFINED

#include "../Window.h"
#include "SkChecksum.h"
#include "SkTDynamicHash.h"

namespace sk_app {

struct ContextPlatformData_mac {
#if 0
    // TODO: use Mac-specific objects
    Display*     fDisplay;
    XWindow      fWindow;
    XVisualInfo* fVisualInfo;
#endif
};

class Window_mac : public Window {
public:
    Window_mac() : Window()
#if 0
                  // TODO: use Mac-specific objects
                  , fDisplay(nullptr)
                  , fWindow(0)
                  , fGC(nullptr)
                  , fVisualInfo(nullptr)
#endif
                  , fMSAASampleCount(0) {}
    ~Window_mac() override { this->closeWindow(); }

#if 0
    // TODO: need to init with Mac-specific data
    bool initWindow(Display* display, const DisplayParams* params);
#endif

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType attachType, const DisplayParams& params) override;

    void onInval() override;

private:
    void closeWindow();

#if 0
    // TODO: use Mac-specific window data
    Display*     fDisplay;
    XWindow      fWindow;
    GC           fGC;
    XVisualInfo* fVisualInfo;
#endif
    
    int          fMSAASampleCount;
};

}   // namespace sk_app

#endif
