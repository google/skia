/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <modules/androidkit/include/SurfaceThread.h>

#include <pthread.h>
#include <android/looper.h>
#include "include/core/SkTypes.h"


SurfaceThread::SurfaceThread() {
    pthread_create(&fThread, nullptr, this->pthread_main, nullptr /* arg */);
}

void SurfaceThread::postMessage(const Message& message) const {
    write(fPipe[1], &message, sizeof(message));
}

void SurfaceThread::readMessage(Message* message) const {
    read(fPipe[0], message, sizeof(Message));
}

int SurfaceThread::message_callback(int /* fd */, int /* events */, void* data) {
    Message message;
    this->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
        case kSurfaceCreated: {
            SkDebugf("surface created");
            break;
        }
        case kSurfaceChanged: {
            SkDebugf("surface created");
            break;
        }
        case kSurfaceDestroyed: {
            SkDebugf("surface created");
            break;
        }
        case kAllSurfacesDestroyed: {
            SkDebugf("surface created");
            return 0;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

void* SurfaceThread::pthread_main(void* /* arg */) {

    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    pipe(fPipe);
    ALooper_addFd(looper, fPipe[0], 1, ALOOPER_EVENT_INPUT,
               this->message_callback, nullptr /* data */);

    while (true) {
        const int ident = ALooper_pollAll(0, nullptr, nullptr, nullptr);

        if (ident >= 0) {
            SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        }
    }
    return nullptr;
}
