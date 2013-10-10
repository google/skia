/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_impl_DEFINED
#define GrGLSL_impl_DEFINED

template<>
inline const char* GrGLSLExpr<4>::ZerosStr() {
    return "vec4(0)";
}

template<>
inline const char* GrGLSLExpr<4>::OnesStr() {
    return "vec4(1)";
}

template<>
inline const char* GrGLSLExpr<4>::ExtractAlphaStr() {
    return "%s.a";
}

template<>
inline const char* GrGLSLExpr<4>::CastStr() {
    return "vec4(%s)";
}
template<>
inline const char* GrGLSLExpr<4>::CastIntStr() {
    return "vec4(%d)";
}

template<>
inline const char* GrGLSLExpr<1>::ZerosStr() {
    return "0";
}

template<>
inline const char* GrGLSLExpr<1>::OnesStr() {
    return "1";
}

// GrGLSLExpr<1>::ExtractAlphaStr() and GrGLSLExpr<1>::CastStr() are
// unimplemented because using them is likely an error. This is now caught
// compile-time.

template<>
inline const char* GrGLSLExpr<1>::CastIntStr() {
    return "%d";
}

template<>
template<>
inline GrGLSLExpr<4> GrGLSLExpr<4>::VectorCast(const GrGLSLExpr<4>& expr) {
    return expr;
}

template<>
template<>
inline GrGLSLExpr<1>  GrGLSLExpr<1>::VectorCast(const GrGLSLExpr<1>& expr)  {
    return expr;
}

template<int N>
template<int M>
inline GrGLSLExpr<N> GrGLSLExpr<N>::VectorCast(const GrGLSLExpr<M>& expr) {
    if (expr.isZeros()) {
        return GrGLSLExpr<N>(0);
    }
    if (expr.isOnes()) {
        return GrGLSLExpr<N>(1);
    }
    return GrGLSLExpr<N>(GrGLSLExpr<N>::CastStr(), expr.c_str());
}

template<int N>
template<int M>
inline GrGLSLExpr<N> GrGLSLExpr<N>::Mul(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1) {
    SK_COMPILE_ASSERT(N == M || M == 1, binary_op_dimensions_incompatible);
    if (in0.isZeros() || in1.isZeros()) {
        return GrGLSLExpr<N>(0);
    }
    if (in0.isOnes()) {
        return VectorCast<M>(in1);
    }
    if (in1.isOnes()) {
        return in0;
    }
    return GrGLSLExpr<N>("(%s * %s)", in0.c_str(), in1.c_str());
}

template<int N>
template<int M>
inline GrGLSLExpr<N> GrGLSLExpr<N>::Add(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1) {
    SK_COMPILE_ASSERT(N == M || M == 1, binary_op_dimensions_incompatible);
    if (in1.isZeros()) {
        return in0;
    }
    if (in0.isZeros()) {
        return VectorCast<M>(in1);
    }
    if (in0.isOnes() && in1.isOnes()) {
        return GrGLSLExpr<N>(2);
    }
    return GrGLSLExpr<N>("(%s + %s)", in0.c_str(), in1.c_str());
}

template<int N>
template<int M>
inline GrGLSLExpr<N> GrGLSLExpr<N>::Sub(const GrGLSLExpr<N>& in0, const GrGLSLExpr<M>& in1) {
    SK_COMPILE_ASSERT(N == M || M == 1, binary_op_dimensions_incompatible);
    if (in1.isZeros()) {
        return in0;
    }
    if (in1.isOnes()) {
        if (in0.isOnes()) {
            return GrGLSLExpr<N>(0);
        }
    }

    return GrGLSLExpr<N>("(%s - %s)", in0.c_str(), in1.c_str());
}

inline GrGLSLExpr<4> GrGLSLExprCast4(const GrGLSLExpr<1>& expr) {
    return GrGLSLExpr<4>::VectorCast(expr);
}

inline GrGLSLExpr<1> GrGLSLExprExtractAlpha(const GrGLSLExpr<4>& expr) {
    if (expr.isZeros()) {
        return GrGLSLExpr<1>(0);
    }
    if (expr.isOnes()) {
        return GrGLSLExpr<1>(1);
    }
    return GrGLSLExpr<1>(GrGLSLExpr<4>::ExtractAlphaStr(), expr.c_str());
}

#endif
