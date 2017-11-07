/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestAtlasTextRenderer_DEFINED
#define TestAtlasTextRenderer_DEFINED

#include "SkAtlasTextRenderer.h"

namespace sk_gpu_test {

class TestAtlasTextRenderer : public SkAtlasTextRenderer {
public:
    virtual void* makeTarget(int width, int height) = 0;
    virtual std::unique_ptr<uint32_t[]> readTarget(void* target) = 0;
};

}  // namespace sk_gpu_test

#endif
