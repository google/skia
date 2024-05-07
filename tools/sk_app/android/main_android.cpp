/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <jni.h>
#include <errno.h>

#include <android_native_app_glue.h>

#include "tools/sk_app/Application.h"
#include "tools/timer/Timer.h"

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
        "/data/local/tmp/skps",
        // TODO: figure out how to use am start with extra params to pass in additional arguments at
        // runtime
        // "--atrace",
    };

    std::unique_ptr<Application> vkApp(Application::Create(std::size(gCmdLine),
                                                           const_cast<char**>(gCmdLine),
                                                           state));

    // loop waiting for stuff to do.
    while (!state->destroyRequested) {
        struct android_poll_source* source = nullptr;
        auto result = ALooper_pollOnce(-1, nullptr, nullptr, (void**)&source);

        if (result == ALOOPER_POLL_ERROR) {
            SkDEBUGFAIL("ALooper_pollOnce returned an error");
        }
        if (source != nullptr) {
            source->process(state, source);
        }
        vkApp->onIdle();
    }
}
//END_INCLUDE(all)
