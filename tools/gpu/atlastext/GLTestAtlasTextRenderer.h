/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLTestAtlasTextRenderer_DEFINED
#define GLTestAtlasTextRenderer_DEFINED

#include "include/core/SkRefCnt.h"

namespace sk_gpu_test {

class TestAtlasTextRenderer;

/**
 * Creates a TestAtlasTextRenderer that uses its own OpenGL context to implement
 * SkAtlasTextRenderer.
 */
sk_sp<TestAtlasTextRenderer> MakeGLTestAtlasTextRenderer();

}  // namespace sk_gpu_test

#endif
