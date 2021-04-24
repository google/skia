/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ANDROIDKIT_SURFACE_H
#define ANDROIDKIT_SURFACE_H

#include <jni.h>

#include "include/core/SkSurface.h"

class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;
    virtual void flushAndSubmit() = 0;

    SkCanvas* getCanvas() const { return fSurface ? fSurface->getCanvas() : nullptr; }

    int width()  const { return fSurface ? fSurface->width()  : 0; }
    int height() const { return fSurface ? fSurface->height() : 0; }

protected:
    sk_sp<SkSurface> fSurface;
};

#endif // ANDROIDKIT_SURFACE_H
