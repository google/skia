/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkTypes.h"
#include "SkTHash.h"
#include "Timer.h"
#include "Window_mac.h"
#include "../Application.h"

using sk_app::Application;

int main(int argc, char**argv) {
#if 0 
    // TODO: use Mac main loop

    Display* display = XOpenDisplay(nullptr);

    Application* app = Application::Create(argc, argv, (void*)display);

    // Get the file descriptor for the X display
    int x11_fd = ConnectionNumber(display);
    fd_set in_fds;

    SkTHashSet<sk_app::Window_mac*> pendingWindows;
    bool done = false;
    while (!done) {
        // Create a file description set containing x11_fd
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        // Set a sleep timer
        struct timeval tv;
        tv.tv_usec = 100;
        tv.tv_sec = 0;

        // Wait for an event on the file descriptor or for timer expiration
        (void) select(1, &in_fds, NULL, NULL, &tv);

        // Handle XEvents (if any) and flush the input 
        XEvent event;
        while (XPending(display) && !done) {
            XNextEvent(display, &event);

            sk_app::Window_mac* win = sk_app::Window_mac::gWindowMap.find(event.xany.window);
            // paint and resize events get collapsed
            switch (event.type) {
            case Expose: 
                win->markPendingPaint();
                pendingWindows.add(win);
                break;
            case ConfigureNotify:
                win->markPendingResize(event.xconfigurerequest.width,
                                       event.xconfigurerequest.height);
                pendingWindows.add(win);
                break;
            default:
                if (win->handleEvent(event)) {
                    done = true;
                }
                break;
            } 
        }
    }

    delete app;

    XCloseDisplay(display);
#endif
    
    return 0;
}
