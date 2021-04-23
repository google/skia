/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/androidkit/src/Surface.h"

class Surface_GL : public SurfaceSurface {
public:
    Surface_GL(JNIEnv* env, jobject surface) : SurfaceSurface(env, surface) {
        fWindow = ANativeWindow_fromSurface(env, surface);
        Message message(kSurfaceCreated);
        message.fNativeWindow = fWindow;
        fThread.postMessage(message);
    }

    void init() override {

    }

    void update() override {

    }

    void destroy() override {

    }
private:
    // context?
};
