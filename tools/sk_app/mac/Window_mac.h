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

#include "GLFW/glfw3.h"

namespace sk_app {

class Window_mac : public Window {
public:
    Window_mac()
            : INHERITED()
            , fWindow(nullptr)
            , fMSAASampleCount(1) {}
    ~Window_mac() override {
        this->closeWindow();
    }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    void closeWindow();

    void resetMouse();

private:
    GLFWwindow*  fWindow;
    int          fMSAASampleCount;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
