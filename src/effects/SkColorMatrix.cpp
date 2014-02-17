/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkColorMatrix.h"

void SkColorMatrix::setIdentity() {
    memset(fMat, 0, sizeof(fMat));
    fMat[kR_Scale] = fMat[kG_Scale] = fMat[kB_Scale] = fMat[kA_Scale] = 1;
}

void SkColorMatrix::setScale(SkScalar rScale, SkScalar gScale, SkScalar bScale,
                             SkScalar aScale) {
    memset(fMat, 0, sizeof(fMat));
    fMat[kR_Scale] = rScale;
    fMat[kG_Scale] = gScale;
    fMat[kB_Scale] = bScale;
    fMat[kA_Scale] = aScale;
}

void SkColorMatrix::postTranslate(SkScalar dr, SkScalar dg, SkScalar db,
                                  SkScalar da) {
    fMat[kR_Trans] += dr;
    fMat[kG_Trans] += dg;
    fMat[kB_Trans] += db;
    fMat[kA_Trans] += da;
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrix::setRotate(Axis axis, SkScalar degrees) {
    SkScalar S, C;

    S = SkScalarSinCos(SkDegreesToRadians(degrees), &C);

    this->setSinCos(axis, S, C);
}

void SkColorMatrix::setSinCos(Axis axis, SkScalar sine, SkScalar cosine) {
    SkASSERT((unsigned)axis < 3);

    static const uint8_t gRotateIndex[] = {
        6, 7, 11, 12,
        0, 10, 2, 12,
        0, 1,  5,  6,
    };
    const uint8_t* index = gRotateIndex + axis * 4;

    this->setIdentity();
    fMat[index[0]] = cosine;
    fMat[index[1]] = sine;
    fMat[index[2]] = -sine;
    fMat[index[3]] = cosine;
}

void SkColorMatrix::preRotate(Axis axis, SkScalar degrees) {
    SkColorMatrix tmp;
    tmp.setRotate(axis, degrees);
    this->preConcat(tmp);
}

void SkColorMatrix::postRotate(Axis axis, SkScalar degrees) {
    SkColorMatrix tmp;
    tmp.setRotate(axis, degrees);
    this->postConcat(tmp);
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrix::setConcat(const SkColorMatrix& matA,
                              const SkColorMatrix& matB) {
    SkScalar    tmp[20];
    SkScalar*   result = fMat;

    if (&matA == this || &matB == this) {
        result = tmp;
    }

    const SkScalar* a = matA.fMat;
    const SkScalar* b = matB.fMat;

    int index = 0;
    for (int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            result[index++] =   SkScalarMul(a[j + 0], b[i + 0]) +
                                SkScalarMul(a[j + 1], b[i + 5]) +
                                SkScalarMul(a[j + 2], b[i + 10]) +
                                SkScalarMul(a[j + 3], b[i + 15]);
        }
        result[index++] =   SkScalarMul(a[j + 0], b[4]) +
                            SkScalarMul(a[j + 1], b[9]) +
                            SkScalarMul(a[j + 2], b[14]) +
                            SkScalarMul(a[j + 3], b[19]) +
                            a[j + 4];
    }

    if (fMat != result) {
        memcpy(fMat, result, sizeof(fMat));
    }
}

///////////////////////////////////////////////////////////////////////////////

static void setrow(SkScalar row[], SkScalar r, SkScalar g, SkScalar b) {
    row[0] = r;
    row[1] = g;
    row[2] = b;
}

static const SkScalar kHueR = 0.213f;
static const SkScalar kHueG = 0.715f;
static const SkScalar kHueB = 0.072f;

void SkColorMatrix::setSaturation(SkScalar sat) {
    memset(fMat, 0, sizeof(fMat));

    const SkScalar R = kHueR * (1 - sat);
    const SkScalar G = kHueG * (1 - sat);
    const SkScalar B = kHueB * (1 - sat);

    setrow(fMat +  0, R + sat, G, B);
    setrow(fMat +  5, R, G + sat, B);
    setrow(fMat + 10, R, G, B + sat);
    fMat[kA_Scale] = 1;
}

static const SkScalar kR2Y = 0.299f;
static const SkScalar kG2Y = 0.587f;
static const SkScalar kB2Y = 0.114f;

static const SkScalar kR2U = -0.16874f;
static const SkScalar kG2U = -0.33126f;
static const SkScalar kB2U = 0.5f;

static const SkScalar kR2V = 0.5f;
static const SkScalar kG2V = -0.41869f;
static const SkScalar kB2V = -0.08131f;

void SkColorMatrix::setRGB2YUV() {
    memset(fMat, 0, sizeof(fMat));

    setrow(fMat +  0, kR2Y, kG2Y, kB2Y);
    setrow(fMat +  5, kR2U, kG2U, kB2U);
    setrow(fMat + 10, kR2V, kG2V, kB2V);
    fMat[kA_Scale] = 1;
}

static const SkScalar kV2R = 1.402f;
static const SkScalar kU2G = -0.34414f;
static const SkScalar kV2G = -0.71414f;
static const SkScalar kU2B = 1.772f;

void SkColorMatrix::setYUV2RGB() {
    memset(fMat, 0, sizeof(fMat));

    setrow(fMat +  0, 1, 0, kV2R);
    setrow(fMat +  5, 1, kU2G, kV2G);
    setrow(fMat + 10, 1, kU2B, 0);
    fMat[kA_Scale] = 1;
}
