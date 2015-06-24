/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSL_impl_DEFINED
#define GrGLSL_impl_DEFINED

template<typename Self>
template<typename T>
inline Self GrGLSLExpr<Self>::VectorCastImpl(const T& expr) {
    if (expr.isZeros()) {
        return Self(0);
    }
    if (expr.isOnes()) {
        return Self(1);
    }
    return Self(Self::CastStr(), expr.c_str());
}

template<typename Self>
template<typename T0, typename T1>
inline Self GrGLSLExpr<Self>::Mul(T0 in0, T1 in1) {
    if (in0.isZeros() || in1.isZeros()) {
        return Self(0);
    }
    if (in0.isOnes()) {
        return Self::VectorCast(in1);
    }
    if (in1.isOnes()) {
        return Self::VectorCast(in0);
    }
    return Self("(%s * %s)", in0.c_str(), in1.c_str());
}

template<typename Self>
template<typename T0, typename T1>
inline Self GrGLSLExpr<Self>::Add(T0 in0, T1 in1) {
    if (in1.isZeros()) {
        return Self::VectorCast(in0);
    }
    if (in0.isZeros()) {
        return Self::VectorCast(in1);
    }
    if (in0.isOnes() && in1.isOnes()) {
        return Self(2);
    }
    return Self("(%s + %s)", in0.c_str(), in1.c_str());
}

template<typename Self>
template<typename T0, typename T1>
inline Self GrGLSLExpr<Self>::Sub(T0 in0, T1 in1) {
    if (in1.isZeros()) {
        return Self::VectorCast(in0);
    }
    if (in1.isOnes()) {
        if (in0.isOnes()) {
            return Self(0);
        }
    }

    return Self("(%s - %s)", in0.c_str(), in1.c_str());
}

template <typename Self>
template <typename T>
T GrGLSLExpr<Self>::extractComponents(const char format[]) const {
    if (this->isZeros()) {
        return T(0);
    }
    if (this->isOnes()) {
        return T(1);
    }
    return T(format, this->c_str());
}

inline GrGLSLExpr1 GrGLSLExpr1::VectorCast(const GrGLSLExpr1& expr) {
    return expr;
}

inline const char* GrGLSLExpr1::ZerosStr() {
    return "0";
}

inline const char* GrGLSLExpr1::OnesStr() {
    return "1.0";
}

// GrGLSLExpr1::CastStr() is unimplemented because using them is likely an
// error. This is now caught compile-time.

inline const char* GrGLSLExpr1::CastIntStr() {
    return "%d";
}

inline GrGLSLExpr1 operator*(const GrGLSLExpr1& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr1::Mul(in0, in1);
}

inline GrGLSLExpr1 operator+(const GrGLSLExpr1& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr1::Add(in0, in1);
}

inline GrGLSLExpr1 operator-(const GrGLSLExpr1& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr1::Sub(in0, in1);
}

inline const char* GrGLSLExpr4::ZerosStr() {
    return "vec4(0)";
}

inline const char* GrGLSLExpr4::OnesStr() {
    return "vec4(1)";
}

inline const char* GrGLSLExpr4::CastStr() {
    return "vec4(%s)";
}

inline const char* GrGLSLExpr4::CastIntStr() {
    return "vec4(%d)";
}

inline GrGLSLExpr4 GrGLSLExpr4::VectorCast(const GrGLSLExpr1& expr) {
    return INHERITED::VectorCastImpl(expr);
}

inline GrGLSLExpr4 GrGLSLExpr4::VectorCast(const GrGLSLExpr4& expr) {
    return expr;
}

inline GrGLSLExpr4::AExpr GrGLSLExpr4::a() const {
    return this->extractComponents<GrGLSLExpr4::AExpr>("%s.a");
}

inline GrGLSLExpr4 operator*(const GrGLSLExpr1& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Mul(in0, in1);
}

inline GrGLSLExpr4 operator+(const GrGLSLExpr1& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Add(in0, in1);
}

inline GrGLSLExpr4 operator-(const GrGLSLExpr1& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Sub(in0, in1);
}

inline GrGLSLExpr4 operator*(const GrGLSLExpr4& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr4::Mul(in0, in1);
}

inline GrGLSLExpr4 operator+(const GrGLSLExpr4& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr4::Add(in0, in1);
}

inline GrGLSLExpr4 operator-(const GrGLSLExpr4& in0, const GrGLSLExpr1& in1) {
    return GrGLSLExpr4::Sub(in0, in1);
}

inline GrGLSLExpr4 operator*(const GrGLSLExpr4& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Mul(in0, in1);
}

inline GrGLSLExpr4 operator+(const GrGLSLExpr4& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Add(in0, in1);
}

inline GrGLSLExpr4 operator-(const GrGLSLExpr4& in0, const GrGLSLExpr4& in1) {
    return GrGLSLExpr4::Sub(in0, in1);
}

#endif
