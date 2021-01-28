/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_mac_DEFINED
#define Window_mac_DEFINED

#include "src/core/SkTDynamicHash.h"
#include "tools/sk_app/Window.h"

#import <Cocoa/Cocoa.h>

namespace sk_app {

class Window_mac : public Window {
public:
    Window_mac()
            : INHERITED()
            , fWindow(nil) {}
    ~Window_mac() override {
        this->closeWindow();
    }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override {}

    float scaleFactor() const override;

    static void PaintWindows();

    static const NSInteger& GetKey(const Window_mac& w) {
        return w.fWindowNumber;
    }

    static uint32_t Hash(const NSInteger& windowNumber) {
        return windowNumber;
    }

    NSWindow* window() { return fWindow; }
    void closeWindow();

private:
    NSWindow*    fWindow;
    NSInteger    fWindowNumber;

    static SkTDynamicHash<Window_mac, NSInteger> gWindowMap;

    using INHERITED = Window;
};

}   // namespace sk_app

#endif
