
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSWindow_SDL_DEFINED
#define SkOSWindow_SDL_DEFINED

#include "SDL.h"
#include "SkWindow.h"

class SkGLCanvas;

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* screen);
    virtual ~SkOSWindow();

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

    void handleSDLEvent(const SDL_Event& event);

protected:
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);
    virtual void onSetTitle(const char[]);

private:
    SDL_Surface* fScreen;
    SDL_Surface* fSurface;
    SkGLCanvas* fGLCanvas;

    void doDraw();

    typedef SkWindow INHERITED;
};

#endif

