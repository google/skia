/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include "SkTArray.h"

/**
 * Types of shader-language-specific boxed variables we can create.
 * (Currently only GrGLShaderVars, but should be applicable to other shader
 * languages.)
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
 * Types used to describe format of vertices in arrays
  */
enum GrVertexAttribType {
    kFloat_GrVertexAttribType = 0,
    kVec2f_GrVertexAttribType,
    kVec3f_GrVertexAttribType,
    kVec4f_GrVertexAttribType,
    kVec4ub_GrVertexAttribType,   // vector of 4 unsigned bytes, e.g. colors

    kLast_GrVertexAttribType = kVec4ub_GrVertexAttribType
};
static const int kGrVertexAttribTypeCount = kLast_GrVertexAttribType + 1;

struct GrVertexAttrib {
    inline void set(GrVertexAttribType type, size_t offset) {
        fType = type; fOffset = offset;
    }
    bool operator==(const GrVertexAttrib& other) const {
        return fType == other.fType && fOffset == other.fOffset;
    };
    bool operator!=(const GrVertexAttrib& other) const { return !(*this == other); }

    GrVertexAttribType fType;
    size_t             fOffset;
};

template <int N>
class GrVertexAttribArray : public SkSTArray<N, GrVertexAttrib, true> {};

#endif
