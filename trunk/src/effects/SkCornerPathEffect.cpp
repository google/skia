
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCornerPathEffect.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkFlattenableBuffers.h"

SkCornerPathEffect::SkCornerPathEffect(SkScalar radius) : fRadius(radius)
{
}

SkCornerPathEffect::~SkCornerPathEffect()
{
}

static bool ComputeStep(const SkPoint& a, const SkPoint& b, SkScalar radius,
                        SkPoint* step) {
    SkScalar dist = SkPoint::Distance(a, b);

    step->set(b.fX - a.fX, b.fY - a.fY);

    if (dist <= radius * 2) {
        step->scale(SK_ScalarHalf);
        return false;
    } else {
        step->scale(SkScalarDiv(radius, dist));
        return true;
    }
}

bool SkCornerPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                    SkStrokeRec*) {
    if (fRadius == 0) {
        return false;
    }

    SkPath::Iter    iter(src, false);
    SkPath::Verb    verb, prevVerb = (SkPath::Verb)-1;
    SkPoint         pts[4];

    bool        closed;
    SkPoint     moveTo, lastCorner;
    SkVector    firstStep, step;
    bool        prevIsValid = true;

    // to avoid warnings
    moveTo.set(0, 0);
    firstStep.set(0, 0);
    lastCorner.set(0, 0);

    for (;;) {
        switch (verb = iter.next(pts, false)) {
            case SkPath::kMove_Verb:
                    // close out the previous (open) contour
                if (SkPath::kLine_Verb == prevVerb) {
                    dst->lineTo(lastCorner);
                }
                closed = iter.isClosedContour();
                if (closed) {
                    moveTo = pts[0];
                    prevIsValid = false;
                } else {
                    dst->moveTo(pts[0]);
                    prevIsValid = true;
                }
                break;
            case SkPath::kLine_Verb: {
                bool drawSegment = ComputeStep(pts[0], pts[1], fRadius, &step);
                // prev corner
                if (!prevIsValid) {
                    dst->moveTo(moveTo + step);
                    prevIsValid = true;
                } else {
                    dst->quadTo(pts[0].fX, pts[0].fY, pts[0].fX + step.fX,
                                pts[0].fY + step.fY);
                }
                if (drawSegment) {
                    dst->lineTo(pts[1].fX - step.fX, pts[1].fY - step.fY);
                }
                lastCorner = pts[1];
                prevIsValid = true;
                break;
            }
            case SkPath::kQuad_Verb:
                // TBD - just replicate the curve for now
                if (!prevIsValid) {
                    dst->moveTo(pts[0]);
                    prevIsValid = true;
                }
                dst->quadTo(pts[1], pts[2]);
                lastCorner = pts[2];
                firstStep.set(0, 0);
                break;
            case SkPath::kCubic_Verb:
                if (!prevIsValid) {
                    dst->moveTo(pts[0]);
                    prevIsValid = true;
                }
                // TBD - just replicate the curve for now
                dst->cubicTo(pts[1], pts[2], pts[3]);
                lastCorner = pts[3];
                firstStep.set(0, 0);
                break;
            case SkPath::kClose_Verb:
                if (firstStep.fX || firstStep.fY) {
                    dst->quadTo(lastCorner.fX, lastCorner.fY,
                                lastCorner.fX + firstStep.fX,
                                lastCorner.fY + firstStep.fY);
                    }
                dst->close();
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }

        if (SkPath::kMove_Verb == prevVerb) {
            firstStep = step;
        }
        prevVerb = verb;
    }
DONE:
    return true;
}

void SkCornerPathEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius);
}

SkCornerPathEffect::SkCornerPathEffect(SkFlattenableReadBuffer& buffer) {
    fRadius = buffer.readScalar();
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkCornerPathEffect)

