/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkColorMatrix.h"
#include "include/private/SkFloatingPoint.h"

enum {
    kR_Scale = 0,
    kG_Scale = 6,
    kB_Scale = 12,
    kA_Scale = 18,

    kR_Trans = 4,
    kG_Trans = 9,
    kB_Trans = 14,
    kA_Trans = 19,
};

static void set_concat(float result[20], const float outer[20], const float inner[20]) {
    float    tmp[20];
    float*   target;

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
        memcpy(result, target, 20 * sizeof(float));
    }
}

void SkColorMatrix::setIdentity() {
    memset(fMat, 0, sizeof(fMat));
    fMat[kR_Scale] = fMat[kG_Scale] = fMat[kB_Scale] = fMat[kA_Scale] = 1;
}

void SkColorMatrix::setScale(float rScale, float gScale, float bScale, float aScale) {
    memset(fMat, 0, sizeof(fMat));
    fMat[kR_Scale] = rScale;
    fMat[kG_Scale] = gScale;
    fMat[kB_Scale] = bScale;
    fMat[kA_Scale] = aScale;
}

void SkColorMatrix::postTranslate(float dr, float dg, float db, float da) {
    fMat[kR_Trans] += dr;
    fMat[kG_Trans] += dg;
    fMat[kB_Trans] += db;
    fMat[kA_Trans] += da;
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrix::setRotate(Axis axis, float degrees) {
    float r = sk_float_degrees_to_radians(degrees);
    this->setSinCos(axis, sk_float_sin(r), sk_float_cos(r));
}

void SkColorMatrix::setSinCos(Axis axis, float sine, float cosine) {
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

void SkColorMatrix::preRotate(Axis axis, float degrees) {
    SkColorMatrix tmp;
    tmp.setRotate(axis, degrees);
    this->preConcat(tmp);
}

void SkColorMatrix::postRotate(Axis axis, float degrees) {
    SkColorMatrix tmp;
    tmp.setRotate(axis, degrees);
    this->postConcat(tmp);
}

void SkColorMatrix::setConcat(const SkColorMatrix& matA, const SkColorMatrix& matB) {
    set_concat(fMat, matA.fMat, matB.fMat);
}

///////////////////////////////////////////////////////////////////////////////

static void setrow(float row[], float r, float g, float b) {
    row[0] = r;
    row[1] = g;
    row[2] = b;
}

static const float kHueR = 0.213f;
static const float kHueG = 0.715f;
static const float kHueB = 0.072f;

void SkColorMatrix::setSaturation(float sat) {
    memset(fMat, 0, sizeof(fMat));

    const float R = kHueR * (1 - sat);
    const float G = kHueG * (1 - sat);
    const float B = kHueB * (1 - sat);

    setrow(fMat +  0, R + sat, G, B);
    setrow(fMat +  5, R, G + sat, B);
    setrow(fMat + 10, R, G, B + sat);
    fMat[kA_Scale] = 1;
}

static const float kR2Y = 0.299f;
static const float kG2Y = 0.587f;
static const float kB2Y = 0.114f;

static const float kR2U = -0.16874f;
static const float kG2U = -0.33126f;
static const float kB2U = 0.5f;

static const float kR2V = 0.5f;
static const float kG2V = -0.41869f;
static const float kB2V = -0.08131f;

void SkColorMatrix::setRGB2YUV() {
    memset(fMat, 0, sizeof(fMat));

    setrow(fMat +  0, kR2Y, kG2Y, kB2Y);
    setrow(fMat +  5, kR2U, kG2U, kB2U);
    setrow(fMat + 10, kR2V, kG2V, kB2V);
    fMat[kA_Scale] = 1;
}

static const float kV2R = 1.402f;
static const float kU2G = -0.34414f;
static const float kV2G = -0.71414f;
static const float kU2B = 1.772f;

void SkColorMatrix::setYUV2RGB() {
    memset(fMat, 0, sizeof(fMat));

    setrow(fMat +  0, 1, 0, kV2R);
    setrow(fMat +  5, 1, kU2G, kV2G);
    setrow(fMat + 10, 1, kU2B, 0);
    fMat[kA_Scale] = 1;
}
