/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShaderContext_DEFINED
#define SkImageShaderContext_DEFINED

#include "SkBitmapController.h"
#include "SkColor.h"
#include "SkColorTable.h"
#include <memory>

// Definition used by SkImageShader.cpp and SkRasterPipeline_opts.h.
// Otherwise, completely uninteresting.

struct SkImageShaderContext {
    std::unique_ptr<SkBitmapController::State> state;

    const void*   pixels;
    SkColorTable* ctable;
    SkColor4f     color4f;
    int           stride;
    int           width;
    int           height;
    float         matrix[9];
    float         x[8];
    float         y[8];
    float         fx[8];
    float         fy[8];
    float         scalex[8];
    float         scaley[8];
};

#endif//SkImageShaderContext_DEFINED
