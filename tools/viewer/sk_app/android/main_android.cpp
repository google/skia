/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <jni.h>
#include <errno.h>

#include <android_native_app_glue.h>

#include "../Application.h"
#include "Timer.h"

using sk_app::Application;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    // Make sure glue isn't stripped.
    app_dummy();

    static const char* gCmdLine[] = {
        "viewer",
        "--skps",
        "/data/local/tmp/skp",
        // TODO: figure out how to use am start with extra params to pass in additional arguments at
        // runtime
        // "--atrace",
    };

    std::unique_ptr<Application> vkApp(Application::Create(SK_ARRAY_COUNT(gCmdLine),
                                                           const_cast<char**>(gCmdLine),
                                                           state));

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // block forever waiting for events.
        while ((ident=ALooper_pollAll(-1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                return;
            }

            vkApp->onIdle();
        }
    }
}
//END_INCLUDE(all)
