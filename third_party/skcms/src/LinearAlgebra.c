/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "LinearAlgebra.h"
#include "PortableMath.h"
#include <float.h>

bool skcms_Matrix4x4_invert(const skcms_Matrix4x4* src, skcms_Matrix4x4* dst) {
    double a00 = (double)src->vals[0][0],
           a01 = (double)src->vals[1][0],
           a02 = (double)src->vals[2][0],
           a03 = (double)src->vals[3][0],
           a10 = (double)src->vals[0][1],
           a11 = (double)src->vals[1][1],
           a12 = (double)src->vals[2][1],
           a13 = (double)src->vals[3][1],
           a20 = (double)src->vals[0][2],
           a21 = (double)src->vals[1][2],
           a22 = (double)src->vals[2][2],
           a23 = (double)src->vals[3][2],
           a30 = (double)src->vals[0][3],
           a31 = (double)src->vals[1][3],
           a32 = (double)src->vals[2][3],
           a33 = (double)src->vals[3][3];

    double b00 = a00*a11 - a01*a10,
           b01 = a00*a12 - a02*a10,
           b02 = a00*a13 - a03*a10,
           b03 = a01*a12 - a02*a11,
           b04 = a01*a13 - a03*a11,
           b05 = a02*a13 - a03*a12,
           b06 = a20*a31 - a21*a30,
           b07 = a20*a32 - a22*a30,
           b08 = a20*a33 - a23*a30,
           b09 = a21*a32 - a22*a31,
           b10 = a21*a33 - a23*a31,
           b11 = a22*a33 - a23*a32;

    double determinant = b00*b11
                       - b01*b10
                       + b02*b09
                       + b03*b08
                       - b04*b07
                       + b05*b06;

    if (determinant == 0) {
        return false;
    }

    double invdet = 1.0 / determinant;
    if (invdet > +(double)FLT_MAX || invdet < -(double)FLT_MAX || !isfinitef_((float)invdet)) {
        return false;
    }

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

    dst->vals[0][0] = (float)( a11*b11 - a12*b10 + a13*b09 );
    dst->vals[1][0] = (float)( a02*b10 - a01*b11 - a03*b09 );
    dst->vals[2][0] = (float)( a31*b05 - a32*b04 + a33*b03 );
    dst->vals[3][0] = (float)( a22*b04 - a21*b05 - a23*b03 );
    dst->vals[0][1] = (float)( a12*b08 - a10*b11 - a13*b07 );
    dst->vals[1][1] = (float)( a00*b11 - a02*b08 + a03*b07 );
    dst->vals[2][1] = (float)( a32*b02 - a30*b05 - a33*b01 );
    dst->vals[3][1] = (float)( a20*b05 - a22*b02 + a23*b01 );
    dst->vals[0][2] = (float)( a10*b10 - a11*b08 + a13*b06 );
    dst->vals[1][2] = (float)( a01*b08 - a00*b10 - a03*b06 );
    dst->vals[2][2] = (float)( a30*b04 - a31*b02 + a33*b00 );
    dst->vals[3][2] = (float)( a21*b02 - a20*b04 - a23*b00 );
    dst->vals[0][3] = (float)( a11*b07 - a10*b09 - a12*b06 );
    dst->vals[1][3] = (float)( a00*b09 - a01*b07 + a02*b06 );
    dst->vals[2][3] = (float)( a31*b01 - a30*b03 - a32*b00 );
    dst->vals[3][3] = (float)( a20*b03 - a21*b01 + a22*b00 );

    for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 4; ++c) {
        if (!isfinitef_(dst->vals[r][c])) {
            return false;
        }
    }
    return true;
}

bool skcms_Matrix3x3_invert(const skcms_Matrix3x3* src, skcms_Matrix3x3* dst) {
    double a00 = (double)src->vals[0][0],
           a01 = (double)src->vals[1][0],
           a02 = (double)src->vals[2][0],
           a10 = (double)src->vals[0][1],
           a11 = (double)src->vals[1][1],
           a12 = (double)src->vals[2][1],
           a20 = (double)src->vals[0][2],
           a21 = (double)src->vals[1][2],
           a22 = (double)src->vals[2][2];

    double b0 = a00*a11 - a01*a10,
           b1 = a00*a12 - a02*a10,
           b2 = a01*a12 - a02*a11,
           b3 = a20,
           b4 = a21,
           b5 = a22;

    double determinant = b0*b5
                       - b1*b4
                       + b2*b3;

    if (determinant == 0) {
        return false;
    }

    double invdet = 1.0 / determinant;
    if (!isfinitef_((float)invdet)) {
        return false;
    }

    b0 *= invdet;
    b1 *= invdet;
    b2 *= invdet;
    b3 *= invdet;
    b4 *= invdet;
    b5 *= invdet;

    dst->vals[0][0] = (float)( a11*b5 - a12*b4 );
    dst->vals[1][0] = (float)( a02*b4 - a01*b5 );
    dst->vals[2][0] = (float)(        +     b2 );
    dst->vals[0][1] = (float)( a12*b3 - a10*b5 );
    dst->vals[1][1] = (float)( a00*b5 - a02*b3 );
    dst->vals[2][1] = (float)(        -     b1 );
    dst->vals[0][2] = (float)( a10*b4 - a11*b3 );
    dst->vals[1][2] = (float)( a01*b3 - a00*b4 );
    dst->vals[2][2] = (float)(        +     b0 );

    for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c) {
        if (!isfinitef_(dst->vals[r][c])) {
            return false;
        }
    }
    return true;
}

skcms_Vector4 skcms_Matrix4x4_Vector4_mul(const skcms_Matrix4x4* m, const skcms_Vector4* v) {
    skcms_Vector4 dst = {{0,0,0,0}};
    for (int row = 0; row < 4; ++row) {
        dst.vals[row] = m->vals[row][0] * v->vals[0]
                      + m->vals[row][1] * v->vals[1]
                      + m->vals[row][2] * v->vals[2]
                      + m->vals[row][3] * v->vals[3];
    }
    return dst;
}
