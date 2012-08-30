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
class SkString;

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

enum GrSLConstantVec {
    kZeros_GrSLConstantVec,
    kOnes_GrSLConstantVec,
    kNone_GrSLConstantVec,
};

namespace {
inline int GrSLTypeToVecLength(GrSLType type) {
    static const int kVecLengths[] = {
        0, // kVoid_GrSLType
        1, // kFloat_GrSLType
        2, // kVec2f_GrSLType
        3, // kVec3f_GrSLType
        4, // kVec4f_GrSLType
        1, // kMat33f_GrSLType
        1, // kMat44f_GrSLType
        1, // kSampler2D_GrSLType
    };
    GrAssert((size_t) type < GR_ARRAY_COUNT(kVecLengths));
    return kVecLengths[type];
}

const char* GrGLSLOnesVecf(int count) {
    static const char* kONESVEC[] = {"ERROR", "1.0", "vec2(1,1)",
                                     "vec3(1,1,1)", "vec4(1,1,1,1)"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(kONESVEC));
    return kONESVEC[count];
}

const char* GrGLSLZerosVecf(int count) {
    static const char* kZEROSVEC[] = {"ERROR", "0.0", "vec2(0,0)",
                                      "vec3(0,0,0)", "vec4(0,0,0,0)"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(kZEROSVEC));
    return kZEROSVEC[count];
}
}

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
    with the given number of coordinates, e.g. 2 -> ".y", 3 -> ".z" */
const char* GrGLSLVectorHomogCoord(int count);
const char* GrGLSLVectorHomogCoord(GrSLType type);

/** Return the GLSL swizzle operator for a nonhomogenous components of a vector
    with the given number of coordinates, e.g. 2 -> ".x", 3 -> ".xy" */
const char* GrGLSLVectorNonhomogCoords(int count);
const char* GrGLSLVectorNonhomogCoords(GrSLType type);

/**
  * Produces a string that is the result of modulating two inputs. The inputs must be vec4 or
  * float. The result is always a vec4. The inputs may be expressions, not just identifier names.
  * Either can be NULL or "" in which case the default params control whether vec4(1,1,1,1) or
  * vec4(0,0,0,0) is assumed. It is an error to pass kNone for default<i> if in<i> is NULL or "".
  * Note that when if function determines that the result is a zeros or ones vec then any expression
  * represented by in0 or in1 will not be emitted. The return value indicates whether a zeros, ones
  * or neither was appeneded.
  */
GrSLConstantVec GrGLSLModulate4f(SkString* outAppend,
                                 const char* in0,
                                 const char* in1,
                                 GrSLConstantVec default0 = kOnes_GrSLConstantVec,
                                 GrSLConstantVec default1 = kOnes_GrSLConstantVec);

/**
 * Does an inplace mul, *=, of vec4VarName by mulFactor. If mulFactorDefault is not kNone then
 * mulFactor may be either "" or NULL. In this case either nothing will be appened (kOnes) or an
 * assignment of vec(0,0,0,0) will be appended (kZeros). The assignment is prepended by tabCnt tabs.
 * A semicolon and newline are added after the assignment. (TODO: Remove tabCnt when we auto-insert
 * tabs to custom stage-generated lines.) If a zeros vec is assigned then the return value is
 * kZeros, otherwise kNone.
 */
GrSLConstantVec GrGLSLMulVarBy4f(SkString* outAppend,
                                 int tabCnt,
                                 const char* vec4VarName,
                                 const char* mulFactor,
                                 GrSLConstantVec mulFactorDefault = kOnes_GrSLConstantVec);

/**
  * Produces a string that is the result of adding two inputs. The inputs must be vec4 or float.
  * The result is always a vec4. The inputs may be expressions, not just identifier names. Either
  * can be NULL or "" in which case if the default is kZeros then vec4(0,0,0,0) is assumed. It is an
  * error to pass kOnes for either default or to pass kNone for default<i> if in<i> is NULL or "".
  * Note that if the function determines that the result is a zeros vec any expression represented
  * by in0 or in1 will not be emitted. The return value indicates whether a zeros vec was appended
  * or not.
  */
GrSLConstantVec GrGLSLAdd4f(SkString* outAppend,
                            const char* in0,
                            const char* in1,
                            GrSLConstantVec default0 = kZeros_GrSLConstantVec,
                            GrSLConstantVec default1 = kZeros_GrSLConstantVec);

#endif
