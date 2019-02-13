/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_DEFINED
#define GrGLSL_DEFINED

#include "GrTypesPriv.h"

class GrShaderCaps;

// Limited set of GLSL versions we build shaders for. Caller should round
// down the GLSL version to one of these enums.
enum GrGLSLGeneration {
    /**
     * Desktop GLSL 1.10 and ES2 shading language (based on desktop GLSL 1.20)
     */
    k110_GrGLSLGeneration,
    /**
     * Desktop GLSL 1.30
     */
    k130_GrGLSLGeneration,
    /**
     * Desktop GLSL 1.40
     */
    k140_GrGLSLGeneration,
    /**
     * Desktop GLSL 1.50
     */
    k150_GrGLSLGeneration,
    /**
     * Desktop GLSL 3.30, and ES GLSL 3.00
     */
    k330_GrGLSLGeneration,
    /**
     * Desktop GLSL 4.00
     */
    k400_GrGLSLGeneration,
    /**
     * Desktop GLSL 4.20
     */
    k420_GrGLSLGeneration,
    /**
     * ES GLSL 3.10 only TODO Make GLSLCap objects to make this more granular
     */
    k310es_GrGLSLGeneration,
    /**
     * ES GLSL 3.20
     */
    k320es_GrGLSLGeneration,
};

const char* GrGLSLTypeString(GrSLType);

#endif
