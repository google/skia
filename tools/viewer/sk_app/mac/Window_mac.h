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

#include "SDL.h"

namespace sk_app {

class Window_mac : public Window {
public:
    Window_mac()
        : INHERITED()
        , fWindow(nullptr)
        , fWindowID(0)
        , fMSAASampleCount(0) {}
    ~Window_mac() override { this->closeWindow(); }

    bool initWindow();

    void setTitle(const char*) override;
    void show() override;

    bool attach(BackendType) override;

    void onInval() override;

    static bool HandleWindowEvent(const SDL_Event& event);

    static const Uint32& GetKey(const Window_mac& w) {
        return w.fWindowID;
    }

    static uint32_t Hash(const Uint32& winID) {
        return winID;
    }

private:
    bool handleEvent(const SDL_Event& event);

    void closeWindow();

    static SkTDynamicHash<Window_mac, Uint32> gWindowMap;

    SDL_Window*  fWindow;
    Uint32       fWindowID;
    
    int          fMSAASampleCount;
    
    typedef Window INHERITED;
};

}   // namespace sk_app

#endif
