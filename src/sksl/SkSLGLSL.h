/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLGLSL_DEFINED
#define SkSLGLSL_DEFINED

namespace SkSL {

// Limited set of GLSL versions we build shaders for. Caller should round
// down the GLSL version to one of these enums.
enum class GLSLGeneration {
    /**
     * Desktop GLSL 1.10 and ES2 shading language (based on desktop GLSL 1.20)
     */
    k110,
    /**
     * Desktop GLSL 1.30
     */
    k130,
    /**
     * Desktop GLSL 1.40
     */
    k140,
    /**
     * Desktop GLSL 1.50
     */
    k150,
    /**
     * Desktop GLSL 3.30, and ES GLSL 3.00
     */
    k330,
    /**
     * Desktop GLSL 4.00
     */
    k400,
    /**
     * Desktop GLSL 4.20
     */
    k420,
    /**
     * ES GLSL 3.10 only TODO Make GLSLCap objects to make this more granular
     */
    k310es,
    /**
     * ES GLSL 3.20
     */
    k320es,
};

} // namespace SkSL

#endif
