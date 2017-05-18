/*

* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkTypes.h"
#include "SkTHash.h"
#include "Timer.h"
#include "Window_unix.h"
#include "../Application.h"

using sk_app::Application;

void finishWindow(sk_app::Window_unix* win) {
    win->finishResize();
    win->finishPaint();
}

int main(int argc, char**argv) {

    Display* display = XOpenDisplay(nullptr);

    Application* app = Application::Create(argc, argv, (void*)display);

    // Get the file descriptor for the X display
    int x11_fd = ConnectionNumber(display);
    int count = x11_fd + 1;

    SkTHashSet<sk_app::Window_unix*> pendingWindows;
    bool done = false;
    while (!done) {
        // Create a file description set containing x11_fd
        fd_set in_fds;
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        // Set a sleep timer
        struct timeval tv;
        tv.tv_usec = 100;
        tv.tv_sec = 0;

        while (!XPending(display)) {
            // Wait for an event on the file descriptor or for timer expiration
            (void) select(count, &in_fds, NULL, NULL, &tv);
        }

        // Handle XEvents (if any) and flush the input
        int count = XPending(display);
        while (count-- && !done) {
            XEvent event;
            XNextEvent(display, &event);

            sk_app::Window_unix* win = sk_app::Window_unix::gWindowMap.find(event.xany.window);
            if (!win) {
                continue;
            }

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
        
        pendingWindows.foreach(finishWindow);
        if (pendingWindows.count() > 0) {
            app->onIdle();
        }
        pendingWindows.reset();

        XFlush(display);
    }

    delete app;

    XCloseDisplay(display);

    return 0;
}
