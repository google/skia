/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCubicMap.h"
#include "SkNx.h"
#include "../../src/pathops/SkPathOpsCubic.h"

void SkCubicMap::setPts(SkPoint p1, SkPoint p2) {
    Sk2s s1 = Sk2s::Load(&p1) * 3;
    Sk2s s2 = Sk2s::Load(&p2) * 3;

    s1 = Sk2s::Min(Sk2s::Max(s1, 0), 3);
    s2 = Sk2s::Min(Sk2s::Max(s2, 0), 3);

    (Sk2s(1) + s1 - s2).store(&fCoeff[0]);
    (s2 - s1 - s1).store(&fCoeff[1]);
    s1.store(&fCoeff[2]);

    this->buildXTable();
}

SkPoint SkCubicMap::computeFromT(float t) const {
    Sk2s a = Sk2s::Load(&fCoeff[0]);
    Sk2s b = Sk2s::Load(&fCoeff[1]);
    Sk2s c = Sk2s::Load(&fCoeff[2]);

    SkPoint result;
    (((a * t + b) * t + c) * t).store(&result);
    return result;
}

float SkCubicMap::computeYFromX(float x) const {
    x = SkTPin<float>(x, 0, 0.99999f) * kTableCount;
    float ix = sk_float_floor(x);
    int index = (int)ix;
    SkASSERT((unsigned)index < SK_ARRAY_COUNT(fXTable));
    return this->computeFromT(fXTable[index].fT0 + fXTable[index].fDT * (x - ix)).fY;
}

float SkCubicMap::hackYFromX(float x) const {
    x = SkTPin<float>(x, 0, 0.99999f) * kTableCount;
    float ix = sk_float_floor(x);
    int index = (int)ix;
    SkASSERT((unsigned)index < SK_ARRAY_COUNT(fXTable));
    return fXTable[index].fY0 + fXTable[index].fDY * (x - ix);
}

static float compute_t_from_x(float A, float B, float C, float x) {
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, -x, roots);
    SkASSERT(count == 1);
    return (float)roots[0];
}

void SkCubicMap::buildXTable() {
    float prevT = 0;

    const float dx = 1.0f / kTableCount;
    float x = dx;

    fXTable[0].fT0 = 0;
    fXTable[0].fY0 = 0;
    for (int i = 1; i < kTableCount; ++i) {
        float t = compute_t_from_x(fCoeff[0].fX, fCoeff[1].fX, fCoeff[2].fX, x);
        SkASSERT(t > prevT);

        fXTable[i - 1].fDT = t - prevT;
        fXTable[i].fT0 = t;

        SkPoint p = this->computeFromT(t);
        fXTable[i - 1].fDY = p.fY - fXTable[i - 1].fY0;
        fXTable[i].fY0 = p.fY;

        prevT = t;
        x += dx;
    }
    fXTable[kTableCount - 1].fDT = 1 - prevT;
    fXTable[kTableCount - 1].fDY = 1 - fXTable[kTableCount - 1].fY0;
}
