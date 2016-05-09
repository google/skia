/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef surface_glue_android_DEFINED
#define surface_glue_android_DEFINED

#include <pthread.h>

#include <android/native_window_jni.h>

#include "../Application.h"
#include "../Window.h"

namespace sk_app {

enum MessageType {
    kUndefined,
    kSurfaceCreated,
    kSurfaceChanged,
    kSurfaceDestroyed,
    kDestroyApp,
    kContentInvalidated
};

struct Message {
    MessageType fType = kUndefined;
    ANativeWindow* fNativeWindow = nullptr;

    Message() {}
    Message(MessageType t) : fType(t) {}
};

struct SkiaAndroidApp {
    int fPipes[2];  // 0 is the read message pipe, 1 is the write message pipe
    Application* fApp;
    Window* fWindow;
    ANativeWindow* fNativeWindow;

    SkiaAndroidApp();
    ~SkiaAndroidApp();
    void postMessage(const Message& message);
    void readMessage(Message* message);
    void paintIfNeeded();

private:
    pthread_t fThread;
};

}  // namespace sk_app

#endif
