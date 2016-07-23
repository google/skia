/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkColorMatrix.h"

// To detect if we need to apply clamping after applying a matrix, we check if
// any output component might go outside of [0, 255] for any combination of
// input components in [0..255].
// Each output component is an affine transformation of the input component, so
// the minimum and maximum values are for any combination of minimum or maximum
// values of input components (i.e. 0 or 255).
// E.g. if R' = x*R + y*G + z*B + w*A + t
// Then the maximum value will be for R=255 if x>0 or R=0 if x<0, and the
// minimum value will be for R=0 if x>0 or R=255 if x<0.
// Same goes for all components.
static bool component_needs_clamping(const SkScalar row[5]) {
    SkScalar maxValue = row[4] / 255;
    SkScalar minValue = row[4] / 255;
    for (int i = 0; i < 4; ++i) {
        if (row[i] > 0)
            maxValue += row[i];
        else
            minValue += row[i];
    }
    return (maxValue > 1) || (minValue < 0);
}

bool SkColorMatrix::NeedsClamping(const SkScalar matrix[20]) {
    return component_needs_clamping(matrix)
        || component_needs_clamping(matrix+5)
        || component_needs_clamping(matrix+10)
        || component_needs_clamping(matrix+15);
}

void SkColorMatrix::SetConcat(SkScalar result[20],
                              const SkScalar outer[20], const SkScalar inner[20]) {
    SkScalar    tmp[20];
    SkScalar*   target;

    if (outer == result || inner == result) {
        target = tmp;   // will memcpy answer when we're done into result
    } else {
        target = result;
    }

    int index = 0;
    for (int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            target[index++] =   outer[j + 0] * inner[i + 0] +
                                outer[j + 1] * inner[i + 5] +
                                outer[j + 2] * inner[i + 10] +
                                outer[j + 3] * inner[i + 15];
        }
        target[index++] =   outer[j + 0] * inner[4] +
                            outer[j + 1] * inner[9] +
                            outer[j + 2] * inner[14] +
                            outer[j + 3] * inner[19] +
                            outer[j + 4];
    }

    if (target != result) {
        memcpy(result, target, 20 * sizeof(SkScalar));
    }
}

///////////////////////////////////////////////////////////////////////////////

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

void SkColorMatrix::setConcat(const SkColorMatrix& matA, const SkColorMatrix& matB) {
    SetConcat(fMat, matA.fMat, matB.fMat);
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
