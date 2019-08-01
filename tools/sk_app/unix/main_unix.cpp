/*

* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/unix/Window_unix.h"
#include "tools/timer/Timer.h"

int main(int argc, char**argv) {
    XInitThreads();
    Display* display = XOpenDisplay(nullptr);

    sk_app::Application* app = sk_app::Application::Create(argc, argv, (void*)display);

    // Get the file descriptor for the X display
    const int x11_fd = ConnectionNumber(display);

    bool done = false;
    while (!done) {
        if (0 == XPending(display)) {
            // Only call select() when we have no events.

            // Create a file description set containing x11_fd
            fd_set in_fds;
            FD_ZERO(&in_fds);
            FD_SET(x11_fd, &in_fds);

            // Set a sleep timer
            struct timeval tv;
            tv.tv_usec = 10;
            tv.tv_sec = 0;

            // Wait for an event on the file descriptor or for timer expiration
            (void)select(1, &in_fds, nullptr, nullptr, &tv);
        }

        // Handle all pending XEvents (if any) and flush the input
        // Only handle a finite number of events before finishing resize and paint..
        if (int count = XPending(display)) {
            // collapse any Expose and Resize events.
            SkTHashSet<sk_app::Window_unix*> pendingWindows;
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
            pendingWindows.foreach([](sk_app::Window_unix* win) {
                win->finishResize();
                win->finishPaint();
            });
        } else {
            // We are only really "idle" when the timer went off with zero events.
            app->onIdle();
        }

        XFlush(display);
    }

    delete app;

    XCloseDisplay(display);

    return 0;
}
