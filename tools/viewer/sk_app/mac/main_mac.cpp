/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkTypes.h"
#include "SkTHash.h"
#include "SDL.h"
#include "Timer.h"
#include "Window_mac.h"
#include "../Application.h"

using sk_app::Application;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SkDebugf("Could not initialize SDL!\n");
        return 1;
    }

    Application* app = Application::Create(argc, argv, nullptr);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                // events handled by the windows
                case SDL_WINDOWEVENT:
                case SDL_MOUSEMOTION:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEWHEEL:
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                case SDL_TEXTINPUT:
                    done = sk_app::Window_mac::HandleWindowEvent(event);
                    break;
                    
                case SDL_QUIT:
                    done = true;
                    break;
                    
                default:
                    break;
            }
        }

        app->onIdle();
    }
    delete app;

    SDL_Quit();

    return 0;
}
