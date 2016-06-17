/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCornerPathEffect.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkCornerPathEffect::SkCornerPathEffect(SkScalar radius) : fRadius(radius) {}
SkCornerPathEffect::~SkCornerPathEffect() {}

static bool ComputeStep(const SkPoint& a, const SkPoint& b, SkScalar radius,
                        SkPoint* step) {
    SkScalar dist = SkPoint::Distance(a, b);

    *step = b - a;
    if (dist <= radius * 2) {
        *step *= SK_ScalarHalf;
        return false;
    } else {
        *step *= radius / dist;
        return true;
    }
}

bool SkCornerPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                    SkStrokeRec*, const SkRect*) const {
    if (0 == fRadius) {
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
    step.set(0, 0);
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
            case SkPath::kConic_Verb:
                // TBD - just replicate the curve for now
                if (!prevIsValid) {
                    dst->moveTo(pts[0]);
                    prevIsValid = true;
                }
                dst->conicTo(pts[1], pts[2], iter.conicWeight());
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
                prevIsValid = false;
                break;
            case SkPath::kDone_Verb:
                if (prevIsValid) {
                    dst->lineTo(lastCorner);
                }
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

sk_sp<SkFlattenable> SkCornerPathEffect::CreateProc(SkReadBuffer& buffer) {
    return SkCornerPathEffect::Make(buffer.readScalar());
}

void SkCornerPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fRadius);
}

#ifndef SK_IGNORE_TO_STRING
void SkCornerPathEffect::toString(SkString* str) const {
    str->appendf("SkCornerPathEffect: (");
    str->appendf("radius: %.2f", fRadius);
    str->appendf(")");
}
#endif
