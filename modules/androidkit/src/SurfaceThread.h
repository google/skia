/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <pthread.h>
#include <unistd.h>
#include <android/looper.h>
#include <android/native_window.h>

#include "include/core/SkPictureRecorder.h"

enum MessageType {
    kUndefined,
    kInitialize,
    kDestroy,
    kRenderPicture,
};

struct Message {
    MessageType fType = kUndefined;
    ANativeWindow* fNativeWindow = nullptr;
    SkPicture* fPicture;

    Message() {}
    Message(MessageType t) : fType(t) {}
};

class SurfaceThread {
public:
    SurfaceThread();

    void postMessage(const Message& message) const;
    void readMessage(Message* message) const;

private:
    static void* pthread_main(void* arg);
    static int message_callback(int fd, int events, void* data);

    pthread_t fThread;
    int fPipe[2]; // acts as a Message queue, read from [0] write to [1]
};
