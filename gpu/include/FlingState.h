
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkFlingState_DEFINED
#define SkFlingState_DEFINED

#include "SkScalar.h"
#include "SkPoint.h"

class SkMatrix;

struct FlingState {
    FlingState() : fActive(false) {}

    bool isActive() const { return fActive; }
    void stop() { fActive = false; }

    void reset(float sx, float sy);
    bool evaluateMatrix(SkMatrix* matrix);

private:
    SkPoint     fDirection;
    SkScalar    fSpeed0;
    double      fTime0;
    bool        fActive;
};

class GrAnimateFloat {
public:
    GrAnimateFloat();

    void start(float v0, float v1, float duration);
    bool isActive() const { return fTime0 != 0; }
    void stop() { fTime0 = 0; }

    float evaluate();

private:
    float   fValue0, fValue1, fDuration;
    SkMSec  fTime0;
};

#endif

