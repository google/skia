/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef AndroidKit_SurfaceThread_DEFINED
#define AndroidKit_SurfaceThread_DEFINED

#include <pthread.h>
#include <unistd.h>
#include <android/looper.h>
#include <android/native_window.h>

#include "tools/sk_app/DisplayParams.h"

#include "include/core/SkPictureRecorder.h"

class WindowSurface;

#include "modules/androidkit/src/Surface.h"

enum MessageType {
    kUndefined,
    kInitialize,
    kDestroy,
    kRenderPicture,
};

struct Message {
    MessageType fType = kUndefined;
    ANativeWindow* fNativeWindow = nullptr;
    SkPicture* fPicture = nullptr;
    WindowSurface** fWindowSurface = nullptr;

    Message() {}
    Message(MessageType t) : fType(t) {}
};

class SurfaceThread {
public:
    SurfaceThread();

    void postMessage(const Message& message) const;
    void readMessage(Message* message) const;
    void release();
private:
    static void* pthread_main(void* arg);
    static int message_callback(int fd, int events, void* data);
    // TODO: This has to be static, which is weird now, but fine in a singleton
    // Switch to singleton design or find other way to break out of thread loop
    bool fRunning;

    pthread_t fThread;
    int fPipe[2]; // acts as a Message queue, read from [0] write to [1]
};

#endif
