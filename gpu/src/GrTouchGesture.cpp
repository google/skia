#include "GrTouchGesture.h"
#include "SkMatrix.h"
#include "SkTime.h"

#include <math.h>

static const SkMSec MAX_DBL_TAP_INTERVAL = 300;
static const float MAX_DBL_TAP_DISTANCE = 100;
static const float MAX_JITTER_RADIUS = 2;

// if true, then ignore the touch-move, 'cause its probably just jitter
static bool close_enough_for_jitter(float x0, float y0, float x1, float y1) {
    return  sk_float_abs(x0 - x1) <= MAX_JITTER_RADIUS &&
            sk_float_abs(y0 - y1) <= MAX_JITTER_RADIUS;
}

///////////////////////////////////////////////////////////////////////////////

GrTouchGesture::GrTouchGesture() {
    this->reset();
}

GrTouchGesture::~GrTouchGesture() {
}

void GrTouchGesture::reset() {
    fTouches.reset();
    fState = kEmpty_State;
    fLocalM.reset();
    fGlobalM.reset();

    fLastUpT = SkTime::GetMSecs() - 2*MAX_DBL_TAP_INTERVAL;
    fLastUpP.set(0, 0);
}

void GrTouchGesture::flushLocalM() {
    fGlobalM.postConcat(fLocalM);
    fLocalM.reset();
}

const SkMatrix& GrTouchGesture::localM() {
    if (fFlinger.isActive()) {
        if (!fFlinger.evaluateMatrix(&fLocalM)) {
            this->flushLocalM();
        }
    }
    return fLocalM;
}

void GrTouchGesture::appendNewRec(void* owner, float x, float y) {
    Rec* rec = fTouches.append();
    rec->fOwner = owner;
    rec->fStartX = rec->fPrevX = rec->fLastX = x;
    rec->fStartY = rec->fPrevY = rec->fLastY = y;
    rec->fLastT = rec->fPrevT = SkTime::GetMSecs();
}

void GrTouchGesture::touchBegin(void* owner, float x, float y) {
//    GrPrintf("--- %d touchBegin %p %g %g\n", fTouches.count(), owner, x, y);

    int index = this->findRec(owner);
    if (index >= 0) {
        this->flushLocalM();
        fTouches.removeShuffle(index);
        GrPrintf("---- already exists, removing\n");
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
            fState = kZoom_State;
            break;
        default:
            break;
    }
}

int GrTouchGesture::findRec(void* owner) const {
    for (int i = 0; i < fTouches.count(); i++) {
        if (owner == fTouches[i].fOwner) {
            return i;
        }
    }
    return -1;
}

static float center(float pos0, float pos1) {
    return (pos0 + pos1) * 0.5f;
}

static const float MAX_ZOOM_SCALE = 4;
static const float MIN_ZOOM_SCALE = 0.25f;

float GrTouchGesture::limitTotalZoom(float scale) const {
    // this query works 'cause we know that we're square-scale w/ no skew/rotation
    const float curr = fGlobalM[0];
    
    if (scale > 1 && curr * scale > MAX_ZOOM_SCALE) {
        scale = MAX_ZOOM_SCALE / curr;
    } else if (scale < 1 && curr * scale < MIN_ZOOM_SCALE) {
        scale = MIN_ZOOM_SCALE / curr;
    }
    return scale;
}

void GrTouchGesture::touchMoved(void* owner, float x, float y) {
//    GrPrintf("--- %d touchMoved %p %g %g\n", fTouches.count(), owner, x, y);

    GrAssert(kEmpty_State != fState);

    int index = this->findRec(owner);
    if (index < 0) {
        // not found, so I guess we should add it...
        GrPrintf("---- add missing begin\n");
        this->appendNewRec(owner, x, y);
        index = fTouches.count() - 1;
    }

    Rec& rec = fTouches[index];

    // not sure how valuable this is
    if (fTouches.count() == 2) {
        if (close_enough_for_jitter(rec.fLastX, rec.fLastY, x, y)) {
//            GrPrintf("--- drop touchMove, withing jitter tolerance %g %g\n", rec.fLastX - x, rec.fLastY - y);
            return;
        }
    }

    rec.fPrevX = rec.fLastX; rec.fLastX = x;
    rec.fPrevY = rec.fLastY; rec.fLastY = y;
    rec.fPrevT = rec.fLastT; rec.fLastT = SkTime::GetMSecs();

    switch (fTouches.count()) {
        case 1: {
            float dx = rec.fLastX - rec.fStartX;
            float dy = rec.fLastY - rec.fStartY;
            dx = (float)sk_float_round2int(dx);
            dy = (float)sk_float_round2int(dy);
            fLocalM.setTranslate(dx, dy);
        } break;
        case 2: {
            GrAssert(kZoom_State == fState);
            const Rec& rec0 = fTouches[0];
            const Rec& rec1 = fTouches[1];
            
            float scale = this->computePinch(rec0, rec1);
            scale = this->limitTotalZoom(scale);

            fLocalM.setTranslate(-center(rec0.fStartX, rec1.fStartX),
                                 -center(rec0.fStartY, rec1.fStartY));
            fLocalM.postScale(scale, scale);
            fLocalM.postTranslate(center(rec0.fLastX, rec1.fLastX),
                                  center(rec0.fLastY, rec1.fLastY));
        } break;
        default:
            break;
    }
}

void GrTouchGesture::touchEnd(void* owner) {
//    GrPrintf("--- %d touchEnd   %p\n", fTouches.count(), owner);

    int index = this->findRec(owner);
    if (index < 0) {
        GrPrintf("--- not found\n");
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
            float dur = (rec.fLastT - rec.fPrevT) * 0.001f;
            if (dur > 0) {
                fFlinger.reset(dx / dur, dy / dur);
            }
            fState = kEmpty_State;
        } break;
        case 2:
            this->flushLocalM();
            GrAssert(kZoom_State == fState);
            fState = kEmpty_State;
            break;
        default:
            GrAssert(kZoom_State == fState);
            break;
    }

    fTouches.removeShuffle(index);
}

float GrTouchGesture::computePinch(const Rec& rec0, const Rec& rec1) {
    double dx = rec0.fStartX - rec1.fStartX;
    double dy = rec0.fStartY - rec1.fStartY;
    double dist0 = sqrt(dx*dx + dy*dy);

    dx = rec0.fLastX - rec1.fLastX;
    dy = rec0.fLastY - rec1.fLastY;
    double dist1 = sqrt(dx*dx + dy*dy);

    double scale = dist1 / dist0;
    return (float)scale;
}

bool GrTouchGesture::handleDblTap(float x, float y) {
    bool found = false;
    SkMSec now = SkTime::GetMSecs();
    if (now - fLastUpT <= MAX_DBL_TAP_INTERVAL) {
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

    fLastUpT = now;
    fLastUpP.set(x, y);
    return found;
}


