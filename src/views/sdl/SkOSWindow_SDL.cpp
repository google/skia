/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOSWindow_SDL.h"
#include "SkCanvas.h"

#include <GL/gl.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static void handle_error() {
    const char* error = SDL_GetError();
    SkDebugf("SDL Error: %s\n", error);
    SDL_ClearError();
}

SkOSWindow::SkOSWindow(void* screen) : fQuit(false) , fGLContext(nullptr) {
    //Create window
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS);
    fWindow = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN );
    if (!fWindow) {
        handle_error();
        return;
    }
    SDL_StartTextInput();
    this->resize(SCREEN_WIDTH, SCREEN_HEIGHT);
}

SkOSWindow::~SkOSWindow() {
    if (fGLContext) {
        SDL_GL_DeleteContext(fGLContext);
    }

    //Destroy window
    SDL_DestroyWindow(fWindow);

    //Quit SDL subsystems
    SDL_Quit();
}

void SkOSWindow::detach() {
    if (fGLContext) {
        SDL_GL_DeleteContext(fGLContext);
        fGLContext = nullptr;
    }
}

bool SkOSWindow::attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo*) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (msaaSampleCount > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaaSampleCount);
    }

    fGLContext = SDL_GL_CreateContext(fWindow);
    if (!fGLContext) {
        handle_error();
        return false;
    }

    int success =  SDL_GL_MakeCurrent(fWindow, fGLContext);
    if (success != 0) {
        handle_error();
        return false;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(1, 1, 1, 1);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    return true;
}

void SkOSWindow::present() {
    SDL_GL_SwapWindow(fWindow);
}

bool SkOSWindow::makeFullscreen() {
    SDL_SetWindowFullscreen(fWindow, SDL_WINDOW_FULLSCREEN);
    return true;
}

void SkOSWindow::setVsync(bool vsync) {
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

void SkOSWindow::closeWindow() {
    fQuit = true;
}

static SkKey convert_sdlkey_to_skkey(SDL_Keycode src) {
    switch (src) {
        case SDLK_UP:
            return kUp_SkKey;
        case SDLK_DOWN:
            return kDown_SkKey;
        case SDLK_LEFT:
            return kLeft_SkKey;
        case SDLK_RIGHT:
            return kRight_SkKey;
        case SDLK_HOME:
            return kHome_SkKey;
        case SDLK_END:
            return kEnd_SkKey;
        case SDLK_ASTERISK:
            return kStar_SkKey;
        case SDLK_HASH:
            return kHash_SkKey;
        case SDLK_0:
            return k0_SkKey;
        case SDLK_1:
            return k1_SkKey;
        case SDLK_2:
            return k2_SkKey;
        case SDLK_3:
            return k3_SkKey;
        case SDLK_4:
            return k4_SkKey;
        case SDLK_5:
            return k5_SkKey;
        case SDLK_6:
            return k6_SkKey;
        case SDLK_7:
            return k7_SkKey;
        case SDLK_8:
            return k8_SkKey;
        case SDLK_9:
            return k9_SkKey;
        default:
            return kNONE_SkKey;
    }
}

void SkOSWindow::handleEvents() {
    SkEvent::ServiceQueueTimer();
    SkEvent::ProcessEvent();

    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (event.motion.state == SDL_PRESSED) {
                    this->handleClick(event.motion.x, event.motion.y,
                                     SkView::Click::kMoved_State, nullptr);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                this->handleClick(event.button.x, event.button.y,
                                  event.button.state == SDL_PRESSED ?
                                  SkView::Click::kDown_State :
                                  SkView::Click::kUp_State, nullptr);
                break;
            case SDL_KEYDOWN: {
                SDL_Keycode key = event.key.keysym.sym;
                SkKey sk = convert_sdlkey_to_skkey(key);
                if (kNONE_SkKey != sk) {
                    if (event.key.state == SDL_PRESSED) {
                        this->handleKey(sk);
                    } else {
                        this->handleKeyUp(sk);
                    }
                } else if (key == SDLK_ESCAPE) {
                    fQuit = true;
                }
                break;
            }
            case SDL_TEXTINPUT: {
                size_t len = strlen(event.text.text);
                for (size_t i = 0; i < len; i++) {
                    this->handleChar((SkUnichar)event.text.text[i]);
                }
                break;
            }
            case SDL_QUIT:
                fQuit = true;
                break;
            default:
                break;
        }
    }
}


void SkOSWindow::onSetTitle(const char title[]) {
    SDL_SetWindowTitle(fWindow, title);
}
///////////////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue() {
    // nothing to do, since we spin on our event-queue
}

void SkEvent::SignalQueueTimer(SkMSec delay) {
    // just need to record the delay time. We handle waking up for it in
}

//////////////////////////////////////////////////////////////////////////////////////////////


#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"

int main(int argc, char** argv){
    SkOSWindow* window = create_sk_window(nullptr, argc, argv);

    // drain any events that occurred before |window| was assigned.
    while (SkEvent::ProcessEvent());

    // Start normal Skia sequence
    application_init();

    window->loop();

    delete window;
    application_term();
    return 0;
}
