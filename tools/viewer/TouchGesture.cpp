/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/TouchGesture.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkFloatingPoint.h"
#include "src/core/SkTime.h"

#include <cmath>

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
    const float speed = fSpeed0 * (std::exp(- K0 * t) - K1);
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

static const float MAX_JITTER_RADIUS = 2;

// Time constant (in seconds) used to smooth the velocity of a moving touch.
static const float VELOCITY_SMOOTHING_SECS = 0.04f;

// Releasing a single dragged touch only starts a fling (inertia) if the touch moved at least
// MIN_FLING_DRAG_DISTANCE pixels from where it started, was still moving within the last
// MAX_FLING_RELEASE_DELAY seconds, and is moving faster than MIN_FLING_SPEED pixels/second. This
// keeps clicks/taps (e.g. a slide selecting whatever is under the cursor), and drags that came to
// rest before being released, from setting the view in motion.
static const float  MIN_FLING_DRAG_DISTANCE = 8;
static const double MAX_FLING_RELEASE_DELAY = 0.05;
static const float  MIN_FLING_SPEED = 100;

// if true, then ignore the touch-move, 'cause its probably just jitter
static bool close_enough_for_jitter(float x0, float y0, float x1, float y1) {
    return  std::fabs(x0 - x1) <= MAX_JITTER_RADIUS &&
            std::fabs(y0 - y1) <= MAX_JITTER_RADIUS;
}

///////////////////////////////////////////////////////////////////////////////

TouchGesture::TouchGesture() {
    this->reset();
}

TouchGesture::~TouchGesture() {
}

void TouchGesture::resetTouchState() {
    fTouches.reset();
    fState = kEmpty_State;
    fLocalM.reset();
}

void TouchGesture::reset() {
    fGlobalM.reset();
    fFlinger.stop();
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
    rec->fStartX = rec->fLastX = x;
    rec->fStartY = rec->fLastY = y;
    rec->fLastT = SkTime::GetSecs();
    rec->fVelX = rec->fVelY = 0;
}

void TouchGesture::touchBegin(void* owner, float x, float y) {
//    SkDebugf("--- %d touchBegin %p %g %g\n", fTouches.count(), owner, x, y);

    int index = this->findRec(owner);
    if (index >= 0) {
        this->flushLocalM();
        fTouches.removeShuffle(index);
        SkDebugf("---- already exists, removing\n");
    }

    if (fTouches.size() == 2) {
        return;
    }

    this->flushLocalM();
    fFlinger.stop();

    this->appendNewRec(owner, x, y);

    switch (fTouches.size()) {
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
    for (int i = 0; i < fTouches.size(); i++) {
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
    this->flushLocalM();
    fFlinger.stop();
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
    if (fTouches.size() == 2) {
        if (close_enough_for_jitter(rec.fLastX, rec.fLastY, x, y)) {
//            SkDebugf("--- drop touchMove, within jitter tolerance %g %g\n", rec.fLastX - x, rec.fLastY - y);
            return;
        }
    }

    const double now = SkTime::GetSecs();
    const float dt = static_cast<float>(now - rec.fLastT);
    if (dt > 0) {
        // Exponentially smooth the velocity. For small dt the weight is roughly
        // dt/VELOCITY_SMOOTHING_SECS, so moves that get handled almost simultaneously (e.g. queued
        // up behind a slow frame) add ~distance/VELOCITY_SMOOTHING_SECS, not a huge distance/dt.
        const float w = 1 - std::exp(-dt / VELOCITY_SMOOTHING_SECS);
        rec.fVelX += w * ((x - rec.fLastX) / dt - rec.fVelX);
        rec.fVelY += w * ((y - rec.fLastY) / dt - rec.fVelY);
    }
    rec.fLastX = x;
    rec.fLastY = y;
    rec.fLastT = now;

    switch (fTouches.size()) {
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

    // count() reflects the number before we removed the owner
    switch (fTouches.size()) {
        case 1: {
            this->flushLocalM();
            // Only fling a drag that is still moving when it's released (see MIN_FLING_SPEED).
            const Rec& rec = fTouches[index];
            if (kTranslate_State == fState &&
                SkPoint::Length(rec.fLastX - rec.fStartX, rec.fLastY - rec.fStartY) >=
                        MIN_FLING_DRAG_DISTANCE &&
                SkTime::GetSecs() - rec.fLastT <= MAX_FLING_RELEASE_DELAY &&
                SkPoint::Length(rec.fVelX, rec.fVelY) >= MIN_FLING_SPEED) {
                fFlinger.reset(rec.fVelX, rec.fVelY);
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
