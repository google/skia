/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
