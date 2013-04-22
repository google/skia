/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_DEFINED
#define GrGLSL_DEFINED

#include "gl/GrGLInterface.h"
#include "GrColor.h"
#include "GrTypesPriv.h"

class GrGLShaderVar;
class SkString;

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
};

enum GrSLConstantVec {
    kZeros_GrSLConstantVec,
    kOnes_GrSLConstantVec,
    kNone_GrSLConstantVec,
};

namespace {
static inline int GrSLTypeToVecLength(GrSLType type) {
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
    GR_STATIC_ASSERT(kGrSLTypeCount == GR_ARRAY_COUNT(kVecLengths));
    return kVecLengths[type];
}

static inline const char* GrGLSLOnesVecf(int count) {
    static const char* kONESVEC[] = {"ERROR", "1.0", "vec2(1,1)",
                                     "vec3(1,1,1)", "vec4(1,1,1,1)"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(kONESVEC));
    return kONESVEC[count];
}

static inline const char* GrGLSLZerosVecf(int count) {
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
 * Returns a string to include at the beginning of a shader to declare the GLSL
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
/**
 * Converts a GrSLType to a string containing the name of the equivalent GLSL type.
 */
static const char* GrGLSLTypeString(GrSLType t) {
    switch (t) {
        case kVoid_GrSLType:
            return "void";
        case kFloat_GrSLType:
            return "float";
        case kVec2f_GrSLType:
            return "vec2";
        case kVec3f_GrSLType:
            return "vec3";
        case kVec4f_GrSLType:
            return "vec4";
        case kMat33f_GrSLType:
            return "mat3";
        case kMat44f_GrSLType:
            return "mat4";
        case kSampler2D_GrSLType:
            return "sampler2D";
        default:
            GrCrash("Unknown shader var type.");
            return ""; // suppress warning
    }
}

/** Return the type enum for a vector of floats of length n (1..4),
    e.g. 1 -> "float", 2 -> "vec2", ... */
static inline const char* GrGLSLFloatVectorTypeString(int n) {
    return GrGLSLTypeString(GrSLFloatVectorType(n));
}

/** Return the GLSL swizzle operator for a homogenous component of a vector
    with the given number of coordinates, e.g. 2 -> ".y", 3 -> ".z" */
const char* GrGLSLVectorHomogCoord(int count);
const char* GrGLSLVectorHomogCoord(GrSLType type);

/** Return the GLSL swizzle operator for a nonhomogenous components of a vector
    with the given number of coordinates, e.g. 2 -> ".x", 3 -> ".xy" */
const char* GrGLSLVectorNonhomogCoords(int count);
const char* GrGLSLVectorNonhomogCoords(GrSLType type);

/**
  * Produces a string that is the result of modulating two inputs. The inputs must be vecN or
  * float. The result is always a vecN. The inputs may be expressions, not just identifier names.
  * Either can be NULL or "" in which case the default params control whether a vector of ones or
  * zeros. It is an error to pass kNone for default<i> if in<i> is NULL or "". Note that when the
  * function determines that the result is a zeros or ones vec then any expression represented by
  * or in1 will not be emitted (side effects won't occur). The return value indicates whether a
  * known zeros or ones vector resulted. The output can be suppressed when known vector is produced
  * by passing true for omitIfConstVec.
  */
template <int N>
GrSLConstantVec GrGLSLModulatef(SkString* outAppend,
                                const char* in0,
                                const char* in1,
                                GrSLConstantVec default0 = kOnes_GrSLConstantVec,
                                GrSLConstantVec default1 = kOnes_GrSLConstantVec,
                                bool omitIfConstVec = false);

/**
 * Produces a string that is the result of adding two inputs. The inputs must be vecN or
 * float. The result is always a vecN. The inputs may be expressions, not just identifier names.
 * Either can be NULL or "" in which case the default params control whether a vector of ones or
 * zeros. It is an error to pass kNone for default<i> if in<i> is NULL or "". Note that when the
 * function determines that the result is a zeros or ones vec then any expression represented by
 * or in1 will not be emitted (side effects won't occur). The return value indicates whether a
 * known zeros or ones vector resulted. The output can be suppressed when known vector is produced
 * by passing true for omitIfConstVec.
 */
template <int N>
GrSLConstantVec GrGLSLAddf(SkString* outAppend,
                           const char* in0,
                           const char* in1,
                           GrSLConstantVec default0 = kZeros_GrSLConstantVec,
                           GrSLConstantVec default1 = kZeros_GrSLConstantVec,
                           bool omitIfConstVec = false);

/**
 * Produces a string that is the result of subtracting two inputs. The inputs must be vecN or
 * float. The result is always a vecN. The inputs may be expressions, not just identifier names.
 * Either can be NULL or "" in which case the default params control whether a vector of ones or
 * zeros. It is an error to pass kNone for default<i> if in<i> is NULL or "". Note that when the
 * function determines that the result is a zeros or ones vec then any expression represented by
 * or in1 will not be emitted (side effects won't occur). The return value indicates whether a
 * known zeros or ones vector resulted. The output can be suppressed when known vector is produced
 * by passing true for omitIfConstVec.
 */
template <int N>
GrSLConstantVec GrGLSLSubtractf(SkString* outAppend,
                                const char* in0,
                                const char* in1,
                                GrSLConstantVec default0 = kZeros_GrSLConstantVec,
                                GrSLConstantVec default1 = kZeros_GrSLConstantVec,
                                bool omitIfConstVec = false);

/**
 * Does an inplace mul, *=, of vec4VarName by mulFactor. If mulFactorDefault is not kNone then
 * mulFactor may be either "" or NULL. In this case either nothing will be appended (kOnes) or an
 * assignment of vec(0,0,0,0) will be appended (kZeros). The assignment is prepended by tabCnt tabs.
 * A semicolon and newline are added after the assignment. (TODO: Remove tabCnt when we auto-insert
 * tabs to GrGLEffect-generated lines.) If a zeros vec is assigned then the return value is
 * kZeros, otherwise kNone.
 */
GrSLConstantVec GrGLSLMulVarBy4f(SkString* outAppend,
                                 int tabCnt,
                                 const char* vec4VarName,
                                 const char* mulFactor,
                                 GrSLConstantVec mulFactorDefault = kOnes_GrSLConstantVec);

/**
 * Given an expression that evaluates to a GLSL vec4, extract a component. If expr is NULL or ""
 * the value of defaultExpr is used. It is an error to pass an empty expr and have set defaultExpr
 * to kNone. The return value indicates whether the value is known to be 0 or 1. If omitIfConst is
 * set then nothing is appended when the return is not kNone.
 */
GrSLConstantVec GrGLSLGetComponent4f(SkString* outAppend,
                                     const char* expr,
                                     GrColorComponentFlags component,
                                     GrSLConstantVec defaultExpr = kNone_GrSLConstantVec,
                                     bool omitIfConst = false);

#include "GrGLSL_impl.h"

#endif
