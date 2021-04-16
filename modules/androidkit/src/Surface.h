/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkSurface.h"

class SkSurface;

class Surface {
public:
    virtual ~Surface() = default;

    virtual void swapBuffers() = 0;
    virtual void release(JNIEnv*) = 0;

    SkCanvas* getCanvas() const { return fSurface->getCanvas(); }

protected:
    sk_sp<SkSurface> fSurface;
};
