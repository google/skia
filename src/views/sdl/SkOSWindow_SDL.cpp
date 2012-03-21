
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOSWindow_SDL.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGLCanvas.h"
#include "SkOSMenu.h"
#include "SkTime.h"

static void post_SkEvent_event() {
    SDL_Event evt;
    evt.type = SDL_USEREVENT;
    evt.user.type = SDL_USEREVENT;
    evt.user.code = 0;
    evt.user.data1 = NULL;
    evt.user.data2 = NULL;
    SDL_PushEvent(&evt);
}

static bool skia_setBitmapFromSurface(SkBitmap* dst, SDL_Surface* src) {
    SkBitmap::Config config;
    
    switch (src->format->BytesPerPixel) {
        case 2:
            config = SkBitmap::kRGB_565_Config;
            break;
        case 4:
            config = SkBitmap::kARGB_8888_Config;
            break;
        default:
            return false;
    }
    
    dst->setConfig(config, src->w, src->h, src->pitch);
    dst->setPixels(src->pixels);
    return true;
}

SkOSWindow::SkOSWindow(void* screen) {
    fScreen = reinterpret_cast<SDL_Surface*>(screen);
    this->resize(fScreen->w, fScreen->h);
    
    uint32_t rmask = SK_R32_MASK << SK_R32_SHIFT;
    uint32_t gmask = SK_G32_MASK << SK_G32_SHIFT;
    uint32_t bmask = SK_B32_MASK << SK_B32_SHIFT;
    uint32_t amask = SK_A32_MASK << SK_A32_SHIFT;

    if (fScreen->flags & SDL_OPENGL) {
        fSurface = NULL;
        fGLCanvas = new SkGLCanvas;
        fGLCanvas->setViewport(fScreen->w, fScreen->h);
    } else {
        fGLCanvas = NULL;
        fSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, fScreen->w, fScreen->h,
                                        32, rmask, gmask, bmask, amask);
    }
}

SkOSWindow::~SkOSWindow() {
    delete fGLCanvas;
    if (fSurface) {
        SDL_FreeSurface(fSurface);
    }
}

#include <OpenGL/gl.h>

void SkOSWindow::doDraw() {
    if (fGLCanvas) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glEnable(GL_TEXTURE_2D);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        int count = fGLCanvas->save();
        this->draw(fGLCanvas);
        fGLCanvas->restoreToCount(count);
        SDL_GL_SwapBuffers( );
    } else {
        if ( SDL_MUSTLOCK(fSurface) ) {
            if ( SDL_LockSurface(fSurface) < 0 ) {
                return;
            }
        }

        SkBitmap bitmap;

        if (skia_setBitmapFromSurface(&bitmap, fSurface)) {
            SkCanvas canvas(bitmap);
            this->draw(&canvas);
        }

        if ( SDL_MUSTLOCK(fSurface) ) {
            SDL_UnlockSurface(fSurface);
        }

        int result = SDL_BlitSurface(fSurface, NULL, fScreen, NULL);
        if (result) {
            SkDebugf("------- SDL_BlitSurface returned %d\n", result);
        }
        SDL_UpdateRect(fScreen, 0, 0, fScreen->w, fScreen->h);
    }
}

static SkKey find_skkey(SDLKey src) {
    // this array must match the enum order in SkKey.h
    static const SDLKey gKeys[] = {
        SDLK_UNKNOWN,
        SDLK_UNKNOWN,   // left softkey
        SDLK_UNKNOWN,   // right softkey
        SDLK_UNKNOWN,   // home
        SDLK_UNKNOWN,   // back
        SDLK_UNKNOWN,   // send
        SDLK_UNKNOWN,   // end
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_ASTERISK,
        SDLK_HASH,
        SDLK_UP,
        SDLK_DOWN,
        SDLK_LEFT,
        SDLK_RIGHT,
        SDLK_RETURN,    // OK
        SDLK_UNKNOWN,   // volume up
        SDLK_UNKNOWN,   // volume down
        SDLK_UNKNOWN,   // power
        SDLK_UNKNOWN,   // camera
    };
    
    const SDLKey* array = gKeys;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gKeys); i++) {
        if (array[i] == src) {
            return static_cast<SkKey>(i);
        }
    }
    return kNONE_SkKey;
}

void SkOSWindow::handleSDLEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_VIDEORESIZE:
            this->resize(event.resize.w, event.resize.h);
            break;
        case SDL_VIDEOEXPOSE:
            this->doDraw();
            break;
        case SDL_MOUSEMOTION:
            if (event.motion.state == SDL_PRESSED) {
                this->handleClick(event.motion.x, event.motion.y,
                                   SkView::Click::kMoved_State);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            this->handleClick(event.button.x, event.button.y,
                               event.button.state == SDL_PRESSED ?
                               SkView::Click::kDown_State :
                               SkView::Click::kUp_State);
            break;
        case SDL_KEYDOWN: {
            SkKey sk = find_skkey(event.key.keysym.sym);
            if (kNONE_SkKey != sk) {
                if (event.key.state == SDL_PRESSED) {
                    this->handleKey(sk);
                } else {
                    this->handleKeyUp(sk);
                }
            }
            break;
        }
        case SDL_USEREVENT:
            if (SkEvent::ProcessEvent()) {
                post_SkEvent_event();
            }
            break;
    }
}

void SkOSWindow::onHandleInval(const SkIRect& r) {
    SDL_Event evt;
    evt.type = SDL_VIDEOEXPOSE;
    evt.expose.type = SDL_VIDEOEXPOSE;
    SDL_PushEvent(&evt);
}

void SkOSWindow::onSetTitle(const char title[]) {
    SDL_WM_SetCaption(title, NULL);
}

void SkOSWindow::onAddMenu(const SkOSMenu* sk_menu) {}

///////////////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue() {
    SkDebugf("-------- signal nonempty\n");
    post_SkEvent_event();
}

static Uint32 timer_callback(Uint32 interval) {
//    SkDebugf("-------- timercallback %d\n", interval);
	SkEvent::ServiceQueueTimer();
    return 0;
}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
    SDL_SetTimer(0, NULL);
    if (delay) {
        SDL_SetTimer(delay, timer_callback);
    }
}

