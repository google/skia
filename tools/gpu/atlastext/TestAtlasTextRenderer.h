/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestAtlasTextRenderer_DEFINED
#define TestAtlasTextRenderer_DEFINED

#include "include/atlastext/SkAtlasTextRenderer.h"
#include "include/core/SkBitmap.h"

namespace sk_gpu_test {

class TestContext;

/**
 * Base class for implementations of SkAtlasTextRenderer in order to test the SkAtlasText APIs.
 * Adds a helper for creating SkAtlasTextTargets and to read back the contents of a target as a
 * bitmap.
 */
class TestAtlasTextRenderer : public SkAtlasTextRenderer {
public:
    /** Returns a handle that can be used to construct a SkAtlasTextTarget instance. */
    virtual void* makeTargetHandle(int width, int height) = 0;

    /** Makes a SkBitmap of the target handle's contents. */
    virtual SkBitmap readTargetHandle(void* targetHandle) = 0;

    /** Clears the target to the specified color, encoded as RGBA (low to high byte order) */
    virtual void clearTarget(void* targetHandle, uint32_t color) = 0;
};

}  // namespace sk_gpu_test

#endif
