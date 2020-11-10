/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>

#include "include/core/SkMatrix.h"
#include "include/core/SkTime.h"
#include "tools/viewer/TouchGesture.h"

#define DISCRETIZE_TRANSLATE_TO_AVOID_FLICKER   true

static const SkScalar MAX_FLING_SPEED = SkIntToScalar(1500);

static SkScalar pin_max_fling(SkScalar speed) {
    if (speed > MAX_FLING_SPEED) {
        speed = MAX_FLING_SPEED;
    }
    return speed;
}

static double getseconds() {
    return SkTime::GetMSecs() * 0.001;
}

// returns +1 or -1, depending on the sign of x
// returns +1 if z is zero
static SkScalar SkScalarSignNonZero(SkScalar x) {
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
        unit->fY = SkScalarSignNonZero(unit->fY);
    } else if (SkScalarAbs(unit->fY) < TOLERANCE) {
        unit->fX = SkScalarSignNonZero(unit->fX);
        unit->fY = 0;
    }
}

void TouchGesture::FlingState::reset(float sx, float sy) {
    fActive = true;
    fDirection.set(sx, sy);
    fSpeed0 = SkPoint::Normalize(&fDirection);
    fSpeed0 = pin_max_fling(fSpeed0);
    fTime0 = getseconds();

    unit_axis_align(&fDirection);
//    printf("---- speed %g dir %g %g\n", fSpeed0, fDirection.fX, fDirection.fY);
}

bool TouchGesture::FlingState::evaluateMatrix(SkMatrix* matrix) {
    if (!fActive) {
        return false;
    }

    const float t =  (float)(getseconds() - fTime0);
    const float MIN_SPEED = 2;
    const float K0 = 5;
    const float K1 = 0.02f;
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
        tx = (float)sk_float_round2int(tx);
        ty = (float)sk_float_round2int(ty);
    }
    matrix->setTranslate(tx, ty);
//    printf("---- evaluate (%g %g)\n", tx, ty);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static const SkMSec MAX_DBL_TAP_INTERVAL = 300;
static const float MAX_DBL_TAP_DISTANCE = 100;
static const float MAX_JITTER_RADIUS = 2;

// if true, then ignore the touch-move, 'cause its probably just jitter
static bool close_enough_for_jitter(float x0, float y0, float x1, float y1) {
    return  sk_float_abs(x0 - x1) <= MAX_JITTER_RADIUS &&
            sk_float_abs(y0 - y1) <= MAX_JITTER_RADIUS;
}

///////////////////////////////////////////////////////////////////////////////

TouchGesture::TouchGesture() {
    this->reset();
}

TouchGesture::~TouchGesture() {
}

void TouchGesture::resetTouchState() {
    fIsTransLimited = false;
    fTouches.reset();
    fState = kEmpty_State;
    fLocalM.reset();

    fLastUpMillis = SkTime::GetMSecs() - 2*MAX_DBL_TAP_INTERVAL;
    fLastUpP.set(0, 0);
}

void TouchGesture::reset() {
    fGlobalM.reset();
    this->resetTouchState();
}

void TouchGesture::flushLocalM() {
    fGlobalM.postConcat(fLocalM);
    fLocalM.reset();
}

const SkMatrix& TouchGesture::localM() {
    if (fFlinger.isActive()) {
        if (!fFlinger.evaluateMatrix(&fLocalM)) {
            this->flushLocalM();
        }
    }
    return fLocalM;
}

void TouchGesture::appendNewRec(void* owner, float x, float y) {
    Rec* rec = fTouches.append();
    rec->fOwner = owner;
    rec->fStartX = rec->fPrevX = rec->fLastX = x;
    rec->fStartY = rec->fPrevY = rec->fLastY = y;
    rec->fLastT = rec->fPrevT = static_cast<float>(SkTime::GetSecs());
}

void TouchGesture::touchBegin(void* owner, float x, float y) {
//    SkDebugf("--- %d touchBegin %p %g %g\n", fTouches.count(), owner, x, y);

    int index = this->findRec(owner);
    if (index >= 0) {
        this->flushLocalM();
        fTouches.removeShuffle(index);
        SkDebugf("---- already exists, removing\n");
    }

    if (fTouches.count() == 2) {
        return;
    }

    this->flushLocalM();
    fFlinger.stop();

    this->appendNewRec(owner, x, y);

    switch (fTouches.count()) {
        case 1:
            fState = kTranslate_State;
            break;
        case 2:
            this->startZoom();
            break;
        default:
            break;
    }
}

int TouchGesture::findRec(void* owner) const {
    for (int i = 0; i < fTouches.count(); i++) {
        if (owner == fTouches[i].fOwner) {
            return i;
        }
    }
    return -1;
}

static SkScalar center(float pos0, float pos1) {
    return (pos0 + pos1) * 0.5f;
}

void TouchGesture::startZoom() {
    fState = kZoom_State;
}

void TouchGesture::updateZoom(float scale, float startX, float startY, float lastX, float lastY) {
    fLocalM.setTranslate(-startX, -startY);
    fLocalM.postScale(scale, scale);
    fLocalM.postTranslate(lastX, lastY);
}

void TouchGesture::endZoom() {
    this->flushLocalM();
    SkASSERT(kZoom_State == fState);
    fState = kEmpty_State;
}

void TouchGesture::touchMoved(void* owner, float x, float y) {
//    SkDebugf("--- %d touchMoved %p %g %g\n", fTouches.count(), owner, x, y);

    if (kEmpty_State == fState) {
        return;
    }

    int index = this->findRec(owner);
    if (index < 0) {
        SkDebugf("---- ignoring move without begin\n");
        return;
    }

    Rec& rec = fTouches[index];

    // not sure how valuable this is
    if (fTouches.count() == 2) {
        if (close_enough_for_jitter(rec.fLastX, rec.fLastY, x, y)) {
//            SkDebugf("--- drop touchMove, within jitter tolerance %g %g\n", rec.fLastX - x, rec.fLastY - y);
            return;
        }
    }

    rec.fPrevX = rec.fLastX; rec.fLastX = x;
    rec.fPrevY = rec.fLastY; rec.fLastY = y;
    rec.fPrevT = rec.fLastT;
    rec.fLastT = static_cast<float>(SkTime::GetSecs());

    switch (fTouches.count()) {
        case 1: {
            float dx = rec.fLastX - rec.fStartX;
            float dy = rec.fLastY - rec.fStartY;
            dx = (float)sk_float_round2int(dx);
            dy = (float)sk_float_round2int(dy);
            fLocalM.setTranslate(dx, dy);
        } break;
        case 2: {
            SkASSERT(kZoom_State == fState);
            const Rec& rec0 = fTouches[0];
            const Rec& rec1 = fTouches[1];

            float scale = this->computePinch(rec0, rec1);
            this->updateZoom(scale,
                             center(rec0.fStartX, rec1.fStartX),
                             center(rec0.fStartY, rec1.fStartY),
                             center(rec0.fLastX, rec1.fLastX),
                             center(rec0.fLastY, rec1.fLastY));
        } break;
        default:
            break;
    }
}

void TouchGesture::touchEnd(void* owner) {
//    SkDebugf("--- %d touchEnd   %p\n", fTouches.count(), owner);

    int index = this->findRec(owner);
    if (index < 0) {
        SkDebugf("--- not found\n");
        return;
    }

    const Rec& rec = fTouches[index];
    if (this->handleDblTap(rec.fLastX, rec.fLastY)) {
        return;
    }

    // count() reflects the number before we removed the owner
    switch (fTouches.count()) {
        case 1: {
            this->flushLocalM();
            float dx = rec.fLastX - rec.fPrevX;
            float dy = rec.fLastY - rec.fPrevY;
            float dur = rec.fLastT - rec.fPrevT;
            if (dur > 0) {
                fFlinger.reset(dx / dur, dy / dur);
            }
            fState = kEmpty_State;
        } break;
        case 2:
            this->endZoom();
            break;
        default:
            SkASSERT(kZoom_State == fState);
            break;
    }

    fTouches.removeShuffle(index);

    limitTrans();
}

bool TouchGesture::isFling(SkPoint* dir) {
    if (fFlinger.isActive()) {
        SkScalar speed;
        fFlinger.get(dir, &speed);
        if (speed > 1000) {
            return true;
        }
    }
    return false;
}

float TouchGesture::computePinch(const Rec& rec0, const Rec& rec1) {
    double dx = rec0.fStartX - rec1.fStartX;
    double dy = rec0.fStartY - rec1.fStartY;
    double dist0 = sqrt(dx*dx + dy*dy);

    dx = rec0.fLastX - rec1.fLastX;
    dy = rec0.fLastY - rec1.fLastY;
    double dist1 = sqrt(dx*dx + dy*dy);

    double scale = dist1 / dist0;
    return (float)scale;
}

bool TouchGesture::handleDblTap(float x, float y) {
    bool found = false;
    double now = SkTime::GetMSecs();
    if (now - fLastUpMillis <= MAX_DBL_TAP_INTERVAL) {
        if (SkPoint::Length(fLastUpP.fX - x,
                            fLastUpP.fY - y) <= MAX_DBL_TAP_DISTANCE) {
            fFlinger.stop();
            fLocalM.reset();
            fGlobalM.reset();
            fTouches.reset();
            fState = kEmpty_State;
            found = true;
        }
    }

    fLastUpMillis = now;
    fLastUpP.set(x, y);
    return found;
}

void TouchGesture::setTransLimit(const SkRect& contentRect, const SkRect& windowRect,
                                   const SkMatrix& preTouchMatrix) {
    fIsTransLimited = true;
    fContentRect = contentRect;
    fWindowRect = windowRect;
    fPreTouchM = preTouchMatrix;
}

void TouchGesture::limitTrans() {
    if (!fIsTransLimited) {
        return;
    }

    SkRect scaledContent = fContentRect;
    fPreTouchM.mapRect(&scaledContent);
    fGlobalM.mapRect(&scaledContent);
    const SkScalar ZERO = 0;

    fGlobalM.postTranslate(ZERO, std::min(ZERO, fWindowRect.fBottom - scaledContent.fTop));
    fGlobalM.postTranslate(ZERO, std::max(ZERO, fWindowRect.fTop - scaledContent.fBottom));
    fGlobalM.postTranslate(std::min(ZERO, fWindowRect.fRight - scaledContent.fLeft), ZERO);
    fGlobalM.postTranslate(std::max(ZERO, fWindowRect.fLeft - scaledContent.fRight), ZERO);
}
