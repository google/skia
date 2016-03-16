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
    SkOSWindow(void*);
    virtual ~SkOSWindow();

    enum SkBackEndTypes {
        kNone_BackEndType, // TODO: remove this, it's not a real option.
        kNativeGL_BackEndType,
#if SK_ANGLE
        kANGLE_BackEndType,
#endif // SK_ANGLE
#if SK_COMMAND_BUFFER
        kCommandBuffer_BackEndType,
#endif // SK_COMMAND_BUFFER
    };

    void release();
    bool attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo*);
    void present();
    bool makeFullscreen();
    void setVsync(bool);
    void closeWindow();
    static void RunEventLoop();

protected:
    void onSetTitle(const char title[]) override;

private:
    void createWindow(int msaaSampleCount);
    void destroyWindow();
    void updateWindowTitle();
    static SkOSWindow* GetInstanceForWindowID(Uint32 windowID);
    static bool HasDirtyWindows();
    static void UpdateDirtyWindows();
    static void HandleEvent(const SDL_Event&);

    SDL_Window* fWindow;
    SDL_GLContext fGLContext;
    int fWindowMSAASampleCount;
    typedef SkWindow INHERITED;
};

#endif
