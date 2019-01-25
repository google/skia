/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_mac_DEFINED
#define Window_mac_DEFINED

#include "../Window.h"
//#include "SkChecksum.h"
//#include "SkTDynamicHash.h"

#import <Cocoa/Cocoa.h>

namespace sk_app {

class Window_mac : public Window {
public:
    Window_mac()
            : INHERITED()
            , fWindow(nil)
            , fMainView(nil)
            , fMSAASampleCount(1) {}
    ~Window_mac() override {
        //*** detach view first?
        [fWindow release];
        [fMainView release];
    }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    NSView*      view() { return fMainView; }

private:
    void closeWindow();

    NSWindow*    fWindow;
    NSView*      fMainView;  // can be stored as Window's content view later

    int          fMSAASampleCount;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
