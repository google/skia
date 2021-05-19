/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/androidkit/src/SurfaceThread.h"

#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkTypes.h"

SurfaceThread::SurfaceThread() {
    pipe(fPipe);
    fRunning = true;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

void SurfaceThread::postMessage(const Message& message) const {
    write(fPipe[1], &message, sizeof(message));
}

void SurfaceThread::readMessage(Message* message) const {
    read(fPipe[0], message, sizeof(Message));
}

void SurfaceThread::release() {
    pthread_join(fThread, nullptr);
}

int SurfaceThread::message_callback(int /* fd */, int /* events */, void* data) {
    auto surfaceThread = (SurfaceThread*)data;
    Message message;
    surfaceThread->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
        case kInitialize: {
            sk_app::DisplayParams params;
            auto winctx = sk_app::window_context_factory::MakeGLForAndroid(message.fNativeWindow, params);
            if (!winctx) {
                break;
            }
            *message.fWindowSurface = new WindowSurface(message.fNativeWindow, std::move(winctx));
            break;
        }
        case kDestroy: {
            SkDebugf("surface destroyed, shutting down thread");
            surfaceThread->fRunning = false;
            if(auto* windowSurface = reinterpret_cast<Surface*>(*message.fWindowSurface)){
                windowSurface->release(nullptr);
                delete windowSurface;
            }
            return 0;
            break;
        }
        case kRenderPicture: {
            sk_sp<SkPicture> picture(message.fPicture);
            if(auto* windowSurface = reinterpret_cast<Surface*>(*message.fWindowSurface)){
                windowSurface->getCanvas()->drawPicture(picture);
                windowSurface->flushAndSubmit();
            }
            break;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

void* SurfaceThread::pthread_main(void* arg) {
    auto surfaceThread = (SurfaceThread*)arg;
    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, surfaceThread->fPipe[0], 1, ALOOPER_EVENT_INPUT,
               surfaceThread->message_callback, surfaceThread);

    while (surfaceThread->fRunning) {
        const int ident = ALooper_pollAll(0, nullptr, nullptr, nullptr);

        if (ident >= 0) {
            SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        }
    }
    return nullptr;
}
