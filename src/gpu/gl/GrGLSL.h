/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_DEFINED
#define GrGLSL_DEFINED

#include "gl/GrGLInterface.h"

class GrGLShaderVar;

// Limited set of GLSL versions we build shaders for. Caller should round
// down the GLSL version to one of these enums.
enum GrGLSLGeneration {
    /**
     * Desktop GLSL 1.10 and ES2 shading lang (based on desktop GLSL 1.20)
     */
    k110_GrGLSLGeneration,
    /**
     * Desktop GLSL 1.30
     */
    k130_GrGLSLGeneration,
    /**
     * Dekstop GLSL 1.50
     */
    k150_GrGLSLGeneration,
};

/**
 * Types of shader-language-specific boxed variables we can create.
 * (Currently only GrGLShaderVars, but should be applicable to other shader
 * langauges.)
 */
enum GrSLType {
    kVoid_GrSLType,
    kFloat_GrSLType,
    kVec2f_GrSLType,
    kVec3f_GrSLType,
    kVec4f_GrSLType,
    kMat33f_GrSLType,
    kMat44f_GrSLType,
    kSampler2D_GrSLType
};

/**
 * Gets the most recent GLSL Generation compatible with the OpenGL context.
 */
GrGLSLGeneration GrGetGLSLGeneration(GrGLBinding binding,
                                     const GrGLInterface* gl);

/**
 * Returns a string to include at the begining of a shader to declare the GLSL
 * version.
 */
const char* GrGetGLSLVersionDecl(GrGLBinding binding,
                                 GrGLSLGeneration v);

/**
 * Depending on the GLSL version being emitted there may be an assumed output
 * variable from the fragment shader for the color. Otherwise, the shader must
 * declare an output variable for the color. If this function returns true:
 *    * Parameter var's name will be set to nameIfDeclared
 *    * The variable must be declared in the fragment shader
 *    * The variable has to be bound as the color output
 *      (using glBindFragDataLocation)
 *    If the function returns false:
 *    * Parameter var's name will be set to the GLSL built-in color output name.
 *    * Do not declare the variable in the shader.
 *    * Do not use glBindFragDataLocation to bind the variable
 * In either case var is initialized to represent the color output in the
 * shader.
 */
bool GrGLSLSetupFSColorOuput(GrGLSLGeneration gen,
                             const char* nameIfDeclared,
                             GrGLShaderVar* var);

/** Convert a count of 1..n floats into the corresponding type enum,
    e.g. 1 -> kFloat_GrSLType, 2 -> kVec2_GrSLType, ... */
GrSLType GrSLFloatVectorType(int count);

/** Return the GLSL swizzle operator for a homogenous component of a vector
    with the given number of coordnates, e.g. 2 -> ".y", 3 -> ".z" */
const char* GrGLSLVectorHomogCoord(int count);

/** Return the GLSL swizzle operator for a nonhomogenous components of a vector
    with the given number of coordnates, e.g. 2 -> ".x", 3 -> ".xy" */
const char* GrGLSLVectorNonhomogCoords(int count);

#endif
