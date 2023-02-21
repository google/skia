/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef JetSki_Surface_DEFINED
#define JetSki_Surface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include "tools/window/SkWindowContext.h"

#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

class SurfaceThread;
#include "modules/jetski/src/SurfaceThread.h"

class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;
    virtual void flushAndSubmit() = 0;
    virtual SkCanvas* getCanvas() = 0;

    int width()  const { return fSurface ? fSurface->width()  : 0; }
    int height() const { return fSurface ? fSurface->height() : 0; }

    sk_sp<SkImage> makeImageSnapshot() const {
        return fSurface ? fSurface->makeImageSnapshot() : nullptr;
    }

protected:
    sk_sp<SkSurface> fSurface;
};

class WindowSurface final : public Surface {
public:
    WindowSurface(ANativeWindow* win, std::unique_ptr<SkWindowContext> wctx);

private:
    void release(JNIEnv* env) override;
    SkCanvas* getCanvas() override;
    void flushAndSubmit() override;

    ANativeWindow*                         fWindow;
    std::unique_ptr<SkWindowContext> fWindowContext;
};

class ThreadedSurface final : public Surface {
public:
    ThreadedSurface(JNIEnv* env, jobject surface);

private:
    void release(JNIEnv* env) override;
    SkCanvas* getCanvas() override;
    void flushAndSubmit() override;

    WindowSurface* fWindowSurface = nullptr;
    SkPictureRecorder fRecorder;
    std::unique_ptr<SurfaceThread> fThread;
    int fWidth;
    int fHeight;
};

#endif
