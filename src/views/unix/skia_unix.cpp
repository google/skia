
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "X11/Xlib.h"
#include "X11/keysym.h"

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"
#include "SkTypes.h"

#include <signal.h>
#include <sys/time.h>

SkOSWindow* gWindow;

int main(int argc, char** argv){
    gWindow = create_sk_window(NULL, argc, argv);

    // drain any events that occurred before gWindow was assigned.
    while (SkEvent::ProcessEvent());

    // Start normal Skia sequence
    application_init();

    gWindow->loop();

    delete gWindow;
    application_term();
    return 0;
}

// SkEvent handlers
