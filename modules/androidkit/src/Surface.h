/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "modules/androidkit/src/SurfaceThread.h"

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>


class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;

    SkCanvas* getCanvas() const;

protected:
    sk_sp<SkSurface> fSurface;
};

class SurfaceSurface : public Surface {
public:
    SurfaceSurface(JNIEnv* env, jobject surface);
    void release(JNIEnv*) override;

    // Override when extending SurfaceSurface
    virtual void init() {}
    virtual void update() {}
    virtual void destroy() {}

protected:
    ANativeWindow* fWindow;
    // TODO: Decouple thread from SurfaceSurface if user wants to manage their own thread
    SurfaceThread fThread;
};
