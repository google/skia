/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBlend_DEFINED
#define GrGLBlend_DEFINED

#include "SkXfermode.h"

class GrGLFragmentBuilder;

namespace GrGLBlend {
    /*
     * Appends GLSL code to fsBuilder that assigns a specified blend of the srcColor and dstColor
     * variables to the outColor variable.
     */
    void AppendPorterDuffBlend(GrGLFragmentBuilder* fsBuilder, const char* srcColor,
                               const char* dstColor, const char* outColor, SkXfermode::Mode mode);
};

#endif
