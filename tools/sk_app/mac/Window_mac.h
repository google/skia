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
            , fMainView(nil)
            , fWindow(nil)
            , fMSAASampleCount(1) {}
    ~Window_mac() override {
        [fWindow release];
        [fMainView release];
    }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    NSView*      fMainView;  // can be stored as Window's content view later

private:
    void closeWindow();

    NSWindow*    fWindow;

    int          fMSAASampleCount;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
