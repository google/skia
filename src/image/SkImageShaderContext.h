/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShaderContext_DEFINED
#define SkImageShaderContext_DEFINED

#include "SkColor.h"
class SkColorTable;

// Definition used by SkImageShader.cpp and SkRasterPipeline_opts.h.
// Otherwise, completely uninteresting.

struct SkImageShaderContext {
    const void*   pixels;
    SkColorTable* ctable;
    SkColor4f     color4f;
    int           stride;
    int           width;
    int           height;
    float         matrix[9];
    float         x[8];
    float         y[8];
    float         scale[8];
};

#endif//SkImageShaderContext_DEFINED
