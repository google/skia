/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_SDL_DEFINED
#define SkOSWindow_SDL_DEFINED

#include "SDL.h"
#include "SDL_opengl.h"
#include "SkWindow.h"

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* screen);
    virtual ~SkOSWindow();

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay) {
        SkFAIL("not implemented\n");
        return false;
    }

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
#if SK_ANGLE
        kANGLE_BackEndType,
#endif // SK_ANGLE
#if SK_COMMAND_BUFFER
        kCommandBuffer_BackEndType,
#endif // SK_COMMAND_BUFFER
    };

    void detach();
    bool attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo*);
    void present();
    bool makeFullscreen();
    void setVsync(bool);
    void closeWindow();
    void loop() {
        while (!fQuit) {
            this->handleEvents();
            this->update(nullptr);
        }
    }

protected:
    void onSetTitle(const char title[]) override;
    void onHandleInval(const SkIRect&) override;
    void onPDFSaved(const char title[], const char desc[], const char path[]) override;

private:
    void handleEvents();
    bool fQuit;
    uint32_t fWindowFlags;
    SDL_Window* fWindow;
    SDL_GLContext fGLContext;

    typedef SkWindow INHERITED;
};

#endif
