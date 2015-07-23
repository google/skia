/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <android_native_app_glue.h>

#include "SkApplication.h"
#include "VisualBench.h"

/**
 * Shared state for our app.
 */
enum State {
    kInit_State,
    kAnimate_State,
    kDestroyRequested_State,
    kFinished_State,
};

struct VisualBenchState {
    VisualBenchState() : fApp(NULL), fWindow(NULL), fState(kInit_State) {}
    struct android_app* fApp;
    SkOSWindow* fWindow;
    SkTArray<SkString> fFlags;
    State fState;
};

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct VisualBenchState* state = (struct VisualBenchState*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (state->fApp->window != NULL && kInit_State == state->fState) {
                // drain any events that occurred before |window| was assigned.
                while (SkEvent::ProcessEvent());

                // Start normal Skia sequence
                application_init();

                SkTArray<const char*> args;
                args.push_back("VisualBench");
                for (int i = 0; i < state->fFlags.count(); i++) {
                    SkDebugf(state->fFlags[i].c_str());
                    args.push_back(state->fFlags[i].c_str());
                }

                state->fWindow = create_sk_window((void*)state->fApp->window,
                                                  args.count(),
                                                  const_cast<char**>(args.begin()));
                state->fWindow->forceInvalAll();
                state->fState = kAnimate_State;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            state->fState = kDestroyRequested_State;
            break;
    }
}

void android_main(struct android_app* state) {
    struct VisualBenchState visualBenchState;

    // Make sure glue isn't stripped.
    app_dummy();

    state->userData = &visualBenchState;
    state->onAppCmd = handle_cmd;
    visualBenchState.fApp = state;

    // Get command line arguments
    JavaVM* jvm = state->activity->vm;
    JNIEnv *env;
    jvm->AttachCurrentThread(&env, 0);

    jobject me = state->activity->clazz;

    jclass acl = env->GetObjectClass(me); //class pointer of NativeActivity
    jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
    jobject intent = env->CallObjectMethod(me, giid); //Got our intent

    jclass icl = env->GetObjectClass(intent); //class pointer of Intent
    jmethodID gseid = env->GetMethodID(icl, "getStringExtra",
                                       "(Ljava/lang/String;)Ljava/lang/String;");

    jstring jsParam1 = (jstring)env->CallObjectMethod(intent, gseid,
                                                      env->NewStringUTF("cmdLineFlags"));
    if (jsParam1) {
        const char* flags = env->GetStringUTFChars(jsParam1, 0);
        SkStrSplit(flags, " ", &visualBenchState.fFlags);
        env->ReleaseStringUTFChars(jsParam1, flags);
    }
    jvm->DetachCurrentThread();

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // We loop until all events are read, then continue to draw the next frame of animation.
        while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                return;
            }

        }

        if (visualBenchState.fWindow) {
            if (visualBenchState.fWindow->destroyRequested()) {
                visualBenchState.fState = kDestroyRequested_State;
            } else {
                visualBenchState.fWindow->update(NULL);
            }
        }

        if (kDestroyRequested_State == visualBenchState.fState) {
            SkDELETE(visualBenchState.fWindow);
            visualBenchState.fWindow = NULL;
            application_term();
            ANativeActivity_finish(state->activity);
            visualBenchState.fState = kFinished_State;
        }
    }
}
