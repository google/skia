/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "FlingState.h"
#include "SkMatrix.h"
#include "SkTime.h"

#define DISCRETIZE_TRANSLATE_TO_AVOID_FLICKER   true

static const float MAX_FLING_SPEED = 1500;

static float pin_max_fling(float speed) {
    if (speed > MAX_FLING_SPEED) {
        speed = MAX_FLING_SPEED;
    }
    return speed;
}

static double getseconds() {
    return SkTime::GetMSecs() * 0.001;
}

// returns +1 or -1, depending on the sign of x
// returns +1 if x is zero
static SkScalar SkScalarSign(SkScalar x) {
    SkScalar sign = SK_Scalar1;
    if (x < 0) {
        sign = -sign;
    }
    return sign;
}

static void unit_axis_align(SkVector* unit) {
    const SkScalar TOLERANCE = SkDoubleToScalar(0.15);
    if (SkScalarAbs(unit->fX) < TOLERANCE) {
        unit->fX = 0;
        unit->fY = SkScalarSign(unit->fY);
    } else if (SkScalarAbs(unit->fY) < TOLERANCE) {
        unit->fX = SkScalarSign(unit->fX);
        unit->fY = 0;
    }
}

void FlingState::reset(float sx, float sy) {
    fActive = true;
    fDirection.set(sx, sy);
    fSpeed0 = SkPoint::Normalize(&fDirection);
    fSpeed0 = pin_max_fling(fSpeed0);
    fTime0 = getseconds();

    unit_axis_align(&fDirection);
//    printf("---- speed %g dir %g %g\n", fSpeed0, fDirection.fX, fDirection.fY);
}

bool FlingState::evaluateMatrix(SkMatrix* matrix) {
    if (!fActive) {
        return false;
    }

    const float t =  getseconds() - fTime0;
    const float MIN_SPEED = 2;
    const float K0 = 5.0;
    const float K1 = 0.02;
    const float speed = fSpeed0 * (sk_float_exp(- K0 * t) - K1);
    if (speed <= MIN_SPEED) {
        fActive = false;
        return false;
    }
    float dist = (fSpeed0 - speed) / K0;

//    printf("---- time %g speed %g dist %g\n", t, speed, dist);
    float tx = fDirection.fX * dist;
    float ty = fDirection.fY * dist;
    if (DISCRETIZE_TRANSLATE_TO_AVOID_FLICKER) {
        tx = sk_float_round2int(tx);
        ty = sk_float_round2int(ty);
    }
    matrix->setTranslate(tx, ty);
//    printf("---- evaluate (%g %g)\n", tx, ty);

    return true;
}

////////////////////////////////////////

GrAnimateFloat::GrAnimateFloat() : fTime0(0) {}

void GrAnimateFloat::start(float v0, float v1, float duration) {
    fValue0 = v0;
    fValue1 = v1;
    fDuration = duration;
    if (duration > 0) {
        fTime0 = SkTime::GetMSecs();
        if (!fTime0) {
            fTime0 = 1;  // time0 is our sentinel
        }
    } else {
        fTime0 = 0;
    }
}

float GrAnimateFloat::evaluate() {
    if (!fTime0) {
        return fValue1;
    }
    
    double elapsed = (SkTime::GetMSecs() - fTime0) * 0.001;
    if (elapsed >= fDuration) {
        fTime0 = 0;
        return fValue1;
    }
    
    double t = elapsed / fDuration;
    if (true) {
        t = (3 - 2 * t) * t * t;
    }
    return fValue0 + t * (fValue1 - fValue0);
}


