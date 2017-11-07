/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLTestAtlasTextRenderer_DEFINED
#define GLTestAtlasTextRenderer_DEFINED

#include <memory>

namespace sk_gpu_test {

class TestAtlasTextRenderer;

std::unique_ptr<TestAtlasTextRenderer> MakeGLTestAtlasTextRenderer();

}  // namespace sk_gpu_test

#endif
