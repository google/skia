/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_DEFINED
#define GrGLSL_DEFINED

#include "GrGLInterface.h"

// Limited set of GLSL versions we build shaders for. Caller should round
// down the GLSL version to one of these enums.
enum GrGLSLGeneration {
    /**
     * Desktop GLSL 1.10 and ES2 shading lang (based on desktop GLSL 1.20)
     */
    k110_GLSLGeneration,
    /**
     * Desktop GLSL 1.30
     */
    k130_GLSLGeneration,
    /**
     * Dekstop GLSL 1.50
     */
    k150_GLSLGeneration,
};

GrGLSLGeneration GetGLSLGeneration(GrGLBinding binding,
                                   const GrGLInterface* gl);

#endif

