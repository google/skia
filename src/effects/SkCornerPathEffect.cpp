/* libs/graphics/effects/SkCornerPathEffect.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkCornerPathEffect.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkBuffer.h"

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
                                    SkScalar* width) {
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
        switch (verb = iter.next(pts)) {
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

SkFlattenable::Factory SkCornerPathEffect::getFactory() {
    return CreateProc;
}

void SkCornerPathEffect::flatten(SkFlattenableWriteBuffer& buffer) {
    buffer.writeScalar(fRadius);
}

SkFlattenable* SkCornerPathEffect::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkCornerPathEffect, (buffer));
}

SkCornerPathEffect::SkCornerPathEffect(SkFlattenableReadBuffer& buffer) {
    fRadius = buffer.readScalar();
}

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkCornerPathEffect",
                                     SkCornerPathEffect::CreateProc);

