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
#include "SkString.h"

class GrGLContextInfo;
class GrGLShaderVar;

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

/**
 * Gets the most recent GLSL Generation compatible with the OpenGL context.
 */
GrGLSLGeneration GrGetGLSLGeneration(GrGLBinding binding,
                                     const GrGLInterface* gl);

/**
 * Returns a string to include at the beginning of a shader to declare the GLSL
 * version.
 */
const char* GrGetGLSLVersionDecl(const GrGLContextInfo&);

/**
 * Converts a GrSLType to a string containing the name of the equivalent GLSL type.
 */
static inline const char* GrGLSLTypeString(GrSLType t) {
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

/** A class representing a GLSL expression.
 * The instance can be a variable name, expression or vecN(0) or vecN(1). Does simple constant
 * folding with help of 1 and 0.
 * Complex expressions can be constructed with operators *, +, -
 */
template <int N>
class GrGLSLExpr {
public:
    /** Constructs an invalid expression.
     * Useful only as a return value from functions that never actually return
     * this and instances that will be assigned to later. */
    GrGLSLExpr()
        : fType(kFullExpr_ExprType) {
        SK_COMPILE_ASSERT(N > 0 && N <= 4, dimensions_not_in_range);
        // The only constructor that is allowed to build an empty expression.
        SkASSERT(!this->isValid());
    }

    /** Constructs an expression with all components as value v */
    explicit GrGLSLExpr(int v) {
        SK_COMPILE_ASSERT(N > 0 && N <= 4, dimensions_not_in_range);
        if (v == 0) {
            fType = kZeros_ExprType;
        } else if (v == 1) {
            fType = kOnes_ExprType;
        } else {
            fType = kFullExpr_ExprType;
            fExpr.appendf(CastIntStr(), v);
        }
    }

    /** Constructs an expression from a string.
     * Argument expr is a simple expression or a parenthesized expression. */
    // TODO: make explicit once effects input Exprs.
    GrGLSLExpr(const char expr[]) {
        SK_COMPILE_ASSERT(N > 0 && N <= 4, dimensions_not_in_range);
        if (NULL == expr) {  // TODO: remove this once effects input Exprs.
            fType = kOnes_ExprType;
        } else {
            fType = kFullExpr_ExprType;
            fExpr = expr;
        }
        SkASSERT(this->isValid());
    }

    /** Constructs an expression from a string.
     * Argument expr is a simple expression or a parenthesized expression. */
    // TODO: make explicit once effects input Exprs.
    GrGLSLExpr(const SkString& expr) {
        SK_COMPILE_ASSERT(N > 0 && N <= 4, dimensions_not_in_range);
        if (expr.isEmpty()) {  // TODO: remove this once effects input Exprs.
            fType = kOnes_ExprType;
        } else {
            fType = kFullExpr_ExprType;
            fExpr = expr;
        }
        SkASSERT(this->isValid());
    }

    bool isOnes() const { return kOnes_ExprType == fType; }
    bool isZeros() const { return kZeros_ExprType == fType; }

    const char* c_str() const {
        if (kZeros_ExprType == fType) {
            return ZerosStr();
        } else if (kOnes_ExprType == fType) {
            return OnesStr();
        }
        SkASSERT(!fExpr.isEmpty()); // Empty expressions should not be used.
        return fExpr.c_str();
    }

private:
    GrGLSLExpr(const char format[], const char in0[])
        : fType(kFullExpr_ExprType) {
        fExpr.appendf(format, in0);
    }

    GrGLSLExpr(const char format[], const char in0[], const char in1[])
        : fType(kFullExpr_ExprType) {
        fExpr.appendf(format, in0, in1);
    }

    GrGLSLExpr(const char format[], const char in0[], char in1)
        : fType(kFullExpr_ExprType) {
        fExpr.appendf(format, in0, in1);
    }

    bool isValid() const {
        return kFullExpr_ExprType != fType || !fExpr.isEmpty();
    }

    static const char* ZerosStr();
    static const char* OnesStr();
    static const char* ExtractAlphaStr();
    static const char* CastStr();
    static const char* CastIntStr();

    /** Casts the expression expr into smaller or bigger expression.
     * Casting is done with GLSL rules:
     * M==3, N==4 vec3(a, b, c) -> vec4(a, b, c, 0)
     * N==4, M==3 vec4(a, b, c, d) -> vec3(a, b, c)
     */
    template <int M>
    static GrGLSLExpr<N> VectorCast(const GrGLSLExpr<M>& expr);

    /** GLSL multiplication: component-wise or multiply each component by a scalar.
     * M == N --> vecN(in0.x * in1.x, ...)
     * M == 1 --> vecN(in0.x * in1, ...)
     * otherwise --> compile-time error
     */
    template <int M>
    static GrGLSLExpr<N> Mul(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1);

    /** GLSL addition: component-wise or add a scalar to each compoment.
     * M == N --> vecN(in0.x + in1.x, ...)
     * M == 1 --> vecN(in0.x + in1, ...)
     * otherwise --> compile-time error
     */
    template <int M>
    static GrGLSLExpr<N> Add(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1);

    /** GLSL subtraction: component-wise or subtract compoments by a scalar.
     * M == N --> vecN(in0.x - in1.x, ...)
     * M == 1 --> vecN(in0.x - in1, ...)
     * otherwise --> compile-time error
     */
    template <int M>
    static GrGLSLExpr<N> Sub(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1);

    enum ExprType {
        kZeros_ExprType,
        kOnes_ExprType,
        kFullExpr_ExprType,
    };
    ExprType fType;
    SkString fExpr;

    template <int> friend class GrGLSLExpr;

    /** Multiplies two expressions component-wise. */
    template <int M> friend GrGLSLExpr<M> operator*(const GrGLSLExpr<M>&, const GrGLSLExpr<M>&);
    /** Adds two expressions component-wise. */
    template <int M> friend GrGLSLExpr<M> operator+(const GrGLSLExpr<M>&, const GrGLSLExpr<M>&);
    /** Subtracts two expressions component-wise. */
    template <int M> friend GrGLSLExpr<M> operator-(const GrGLSLExpr<M>&, const GrGLSLExpr<M>&);
    /** Multiplies every component of an expression with a scalar expression. */
    friend GrGLSLExpr<4> operator*(const GrGLSLExpr<4>&, const GrGLSLExpr<1>&);
    /** Adds a scalar expression to every component of an expression. */
    friend GrGLSLExpr<4> operator+(const GrGLSLExpr<4>&, const GrGLSLExpr<1>&);
    /** Subtracts a scalar expression from every component of an expression. */
    friend GrGLSLExpr<4> operator-(const GrGLSLExpr<4>&, const GrGLSLExpr<1>&);

    friend GrGLSLExpr<1> GrGLSLExprExtractAlpha(const GrGLSLExpr<4>& expr);
    friend GrGLSLExpr<4> GrGLSLExprCast4(const GrGLSLExpr<1>& expr);
};


template <int N>
inline GrGLSLExpr<N> operator*(const GrGLSLExpr<N>& in0, const GrGLSLExpr<N>&in1) {
    return GrGLSLExpr<N>::Mul(in0, in1);
}

template <int N>
inline GrGLSLExpr<N> operator+(const GrGLSLExpr<N>& in0, const GrGLSLExpr<N>&in1) {
    return GrGLSLExpr<N>::Add(in0, in1);
}

template <int N>
inline GrGLSLExpr<N> operator-(const GrGLSLExpr<N>& in0, const GrGLSLExpr<N>&in1) {
    return GrGLSLExpr<N>::Sub(in0, in1);
}

inline GrGLSLExpr<4> operator*(const GrGLSLExpr<4>& in0, const GrGLSLExpr<1>& in1) {
    return GrGLSLExpr<4>::Mul(in0, in1);
}

inline GrGLSLExpr<4> operator+(const GrGLSLExpr<4>& in0, const GrGLSLExpr<1>& in1) {
    return GrGLSLExpr<4>::Add(in0, in1);
}

inline GrGLSLExpr<4> operator-(const GrGLSLExpr<4>& in0, const GrGLSLExpr<1>& in1) {
    return GrGLSLExpr<4>::Sub(in0, in1);
}

/** Casts an vec1 expression  to vec4 expresison, eg. vec1(v) -> vec4(v,v,v,v). */
GrGLSLExpr<4> GrGLSLExprCast4(const GrGLSLExpr<1>& expr);

/** Extracts alpha component from an expression of vec<4>. */
GrGLSLExpr<1> GrGLSLExprExtractAlpha(const GrGLSLExpr<4>& expr);

/**
 * Does an inplace mul, *=, of vec4VarName by mulFactor.
 * A semicolon and newline are added after the assignment.
 */
void GrGLSLMulVarBy4f(SkString* outAppend, unsigned tabCnt,
                      const char* vec4VarName, const GrGLSLExpr<4>& mulFactor);

#include "GrGLSL_impl.h"

#endif
