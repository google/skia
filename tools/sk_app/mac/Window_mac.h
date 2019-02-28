/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_mac_DEFINED
#define Window_mac_DEFINED

#include "../Window.h"
#include "SkTDynamicHash.h"

#import <Cocoa/Cocoa.h>

namespace sk_app {

class Window_mac : public Window {
public:
    Window_mac()
            : INHERITED()
            , fWindow(nil)
            , fMSAASampleCount(1)
            , fIsMouseDown(false) {}
    ~Window_mac() override {
        this->closeWindow();
    }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override {}

    static void PaintWindows();
    static void HandleWindowEvent(const NSEvent* event);

    static const NSInteger& GetKey(const Window_mac& w) {
        return w.fWindowNumber;
    }

    static uint32_t Hash(const NSInteger& windowNumber) {
        return windowNumber;
    }

    bool needsPaint() { return this->fIsContentInvalidated; }

    NSWindow* window() { return fWindow; }
    void closeWindow();

    void resetMouse();

private:
    void handleEvent(const NSEvent* event);

    NSWindow*    fWindow;
    NSInteger    fWindowNumber;
    int          fMSAASampleCount;
    bool         fIsMouseDown;
    NSPoint      fMouseDownPos;
    uint32_t     fMouseModifiers;

    static SkTDynamicHash<Window_mac, NSInteger> gWindowMap;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
