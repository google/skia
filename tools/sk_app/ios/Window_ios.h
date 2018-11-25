/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_ios_DEFINED
#define Window_ios_DEFINED

#include "../Window.h"
#include "SkChecksum.h"
#include "SkTDynamicHash.h"

#include "SDL.h"

namespace sk_app {

class Window_ios : public Window {
public:
    Window_ios() : INHERITED(), fWindow(nullptr), fWindowID(0), fMSAASampleCount(1) {}
    ~Window_ios() override { this->closeWindow(); }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    static bool HandleWindowEvent(const SDL_Event& event);

    static const Uint32& GetKey(const Window_ios& w) {
        return w.fWindowID;
    }

    static uint32_t Hash(const Uint32& winID) {
        return winID;
    }

private:
    bool handleEvent(const SDL_Event& event);

    void closeWindow();

    static SkTDynamicHash<Window_ios, Uint32> gWindowMap;

    SDL_Window*  fWindow;
    Uint32       fWindowID;

    int          fMSAASampleCount;

    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
