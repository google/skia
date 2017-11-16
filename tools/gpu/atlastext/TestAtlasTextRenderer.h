/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestAtlasTextRenderer_DEFINED
#define TestAtlasTextRenderer_DEFINED

#include "SkAtlasTextRenderer.h"
#include "SkBitmap.h"

namespace sk_gpu_test {

class TestContext;

class TestAtlasTextRenderer : public SkAtlasTextRenderer {
public:
    /** Returns a handle that can be used to construct a SkAtlasTextTarget instance. */
    virtual void* makeTargetHandle(int width, int height) = 0;

    /** Makes a SkBitmap of the target handle's contents. */
    virtual SkBitmap readTargetHandle(void* targetHandle) = 0;
};

}  // namespace sk_gpu_test

#endif
