/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_win_DEFINED
#define Window_win_DEFINED

#include <windows.h>
#include "../Window.h"

namespace sk_app {

// for Windows
struct ContextPlatformData_win {
    HINSTANCE fHInstance;
    HWND      fHWnd;
};

class Window_win : public Window {
public:
    Window_win() : Window() {}
    ~Window_win() override {}

    bool init(HINSTANCE instance);

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType attachType, const DisplayParams& params) override;

    void onInval() override;

private:
    HINSTANCE fHInstance;
    HWND      fHWnd;
};

}   // namespace sk_app

#endif
