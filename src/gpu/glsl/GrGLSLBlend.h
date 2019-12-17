/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBlend_DEFINED
#define GrGLBlend_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRegion.h"

class GrGLSLFragmentBuilder;

namespace GrGLSLBlend {
    /*
     * Appends GLSL code to fsBuilder that assigns a specified blend of the srcColor and dstColor
     * variables to the outColor variable.
     */
    void AppendMode(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                    const char* dstColor, const char* outColor, SkBlendMode mode);

    void AppendRegionOp(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                        const char* dstColor, const char* outColor, SkRegion::Op regionOp);
};

#endif
