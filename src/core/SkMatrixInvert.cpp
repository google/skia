/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixInvert.h"

#include "include/private/SkFloatingPoint.h"

bool SkInvert2x2Matrix(const SkScalar inMatrix[4], SkScalar outMatrix[4]) {
    double a00 = inMatrix[0];
    double a01 = inMatrix[1];
    double a10 = inMatrix[2];
    double a11 = inMatrix[3];

    // Calculate the inverse determinant
    double det = a00 * a11 - a01 * a10;
    double invdet = sk_ieee_double_divide(1.0, det);

    // If det is zero, we want to return false. However, we also want to return false if 1/det
    // overflows to infinity (i.e. det is denormalized). All of this is subsumed by our final check
    // at the bottom (that all 4 scalar matrix entries are finite).
    outMatrix[0] =  a11 * invdet;
    outMatrix[1] = -a01 * invdet;
    outMatrix[2] = -a10 * invdet;
    outMatrix[3] =  a00 * invdet;
    return SkScalarsAreFinite(outMatrix, 4);
}

bool SkInvert3x3Matrix(const SkScalar inMatrix[9], SkScalar outMatrix[9]) {
    double a00 = inMatrix[0];
    double a01 = inMatrix[1];
    double a02 = inMatrix[2];
    double a10 = inMatrix[3];
    double a11 = inMatrix[4];
    double a12 = inMatrix[5];
    double a20 = inMatrix[6];
    double a21 = inMatrix[7];
    double a22 = inMatrix[8];

    double b01 =  a22 * a11 - a12 * a21;
    double b11 = -a22 * a10 + a12 * a20;
    double b21 =  a21 * a10 - a11 * a20;

    // Calculate the inverse determinant
    double det = a00 * b01 + a01 * b11 + a02 * b21;
    double invdet = sk_ieee_double_divide(1.0, det);

    // If det is zero, we want to return false. However, we also want to return false if 1/det
    // overflows to infinity (i.e. det is denormalized). All of this is subsumed by our final check
    // at the bottom (that all 9 scalar matrix entries are finite).
    outMatrix[0] = b01 * invdet;
    outMatrix[1] = (-a22 * a01 + a02 * a21) * invdet;
    outMatrix[2] = ( a12 * a01 - a02 * a11) * invdet;
    outMatrix[3] = b11 * invdet;
    outMatrix[4] = ( a22 * a00 - a02 * a20) * invdet;
    outMatrix[5] = (-a12 * a00 + a02 * a10) * invdet;
    outMatrix[6] = b21 * invdet;
    outMatrix[7] = (-a21 * a00 + a01 * a20) * invdet;
    outMatrix[8] = ( a11 * a00 - a01 * a10) * invdet;
    return SkScalarsAreFinite(outMatrix, 9);
}

bool SkInvert4x4Matrix(const SkScalar inMatrix[16], SkScalar outMatrix[16]) {
    double a00 = inMatrix[0];
    double a01 = inMatrix[1];
    double a02 = inMatrix[2];
    double a03 = inMatrix[3];
    double a10 = inMatrix[4];
    double a11 = inMatrix[5];
    double a12 = inMatrix[6];
    double a13 = inMatrix[7];
    double a20 = inMatrix[8];
    double a21 = inMatrix[9];
    double a22 = inMatrix[10];
    double a23 = inMatrix[11];
    double a30 = inMatrix[12];
    double a31 = inMatrix[13];
    double a32 = inMatrix[14];
    double a33 = inMatrix[15];

    double b00 = a00 * a11 - a01 * a10;
    double b01 = a00 * a12 - a02 * a10;
    double b02 = a00 * a13 - a03 * a10;
    double b03 = a01 * a12 - a02 * a11;
    double b04 = a01 * a13 - a03 * a11;
    double b05 = a02 * a13 - a03 * a12;
    double b06 = a20 * a31 - a21 * a30;
    double b07 = a20 * a32 - a22 * a30;
    double b08 = a20 * a33 - a23 * a30;
    double b09 = a21 * a32 - a22 * a31;
    double b10 = a21 * a33 - a23 * a31;
    double b11 = a22 * a33 - a23 * a32;

    // Calculate the inverse determinant
    double det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    double invdet = sk_ieee_double_divide(1.0, det);

    // If det is zero, we want to return false. However, we also want to return false if 1/det
    // overflows to infinity (i.e. det is denormalized). All of this is subsumed by our final check
    // at the bottom (that all 16 scalar matrix entries are finite).
    b00 *= invdet;
    b01 *= invdet;
    b02 *= invdet;
    b03 *= invdet;
    b04 *= invdet;
    b05 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;

    outMatrix[0]  = a11 * b11 - a12 * b10 + a13 * b09;
    outMatrix[1]  = a02 * b10 - a01 * b11 - a03 * b09;
    outMatrix[2]  = a31 * b05 - a32 * b04 + a33 * b03;
    outMatrix[3]  = a22 * b04 - a21 * b05 - a23 * b03;
    outMatrix[4]  = a12 * b08 - a10 * b11 - a13 * b07;
    outMatrix[5]  = a00 * b11 - a02 * b08 + a03 * b07;
    outMatrix[6]  = a32 * b02 - a30 * b05 - a33 * b01;
    outMatrix[7]  = a20 * b05 - a22 * b02 + a23 * b01;
    outMatrix[8]  = a10 * b10 - a11 * b08 + a13 * b06;
    outMatrix[9]  = a01 * b08 - a00 * b10 - a03 * b06;
    outMatrix[10] = a30 * b04 - a31 * b02 + a33 * b00;
    outMatrix[11] = a21 * b02 - a20 * b04 - a23 * b00;
    outMatrix[12] = a11 * b07 - a10 * b09 - a12 * b06;
    outMatrix[13] = a00 * b09 - a01 * b07 + a02 * b06;
    outMatrix[14] = a31 * b01 - a30 * b03 - a32 * b00;
    outMatrix[15] = a20 * b03 - a21 * b01 + a22 * b00;
    return SkScalarsAreFinite(outMatrix, 16);
}
