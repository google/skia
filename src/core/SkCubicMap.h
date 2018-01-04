/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCubicMap_DEFINED
#define SkCubicMap_DEFINED

#include "SkPoint.h"

class SkCubicMap {
public:
    void setPts(SkPoint p1, SkPoint p2);
    void setPts(float x1, float y1, float x2, float y2) {
        this->setPts({x1, y1}, {x2, y2});
    }

    SkPoint computeFromT(float t) const;
    float computeYFromX(float x) const;

    // experimental
    float hackYFromX(float x) const;

private:
    SkPoint fCoeff[4];
    // x->t lookup
    enum { kTableCount = 16 };
    struct Rec {
        float   fT0;
        float   fDT;

        float fY0;
        float fDY;
    };
    Rec fXTable[kTableCount];

    void buildXTable();
};
#endif

