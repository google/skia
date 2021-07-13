/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/effects/SkCornerPathEffect.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

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

class SkCornerPathEffectImpl : public SkPathEffectBase {
public:
    explicit SkCornerPathEffectImpl(SkScalar radius) : fRadius(radius) {
        SkASSERT(radius > 0);
    }

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override {
        if (fRadius <= 0) {
            return false;
        }

        SkPath::Iter    iter(src, false);
        SkPath::Verb    verb, prevVerb = SkPath::kDone_Verb;
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
                    return true;
                default:
                    SkDEBUGFAIL("default should not be reached");
                    return false;
            }

            if (SkPath::kMove_Verb == prevVerb) {
                firstStep = step;
            }
            prevVerb = verb;
        }
        return true;
    }

    bool computeFastBounds(SkRect*) const override {
        // Rounding sharp corners within a path produces a new path that is still contained within
        // the original's bounds, so leave 'bounds' unmodified.
        return true;
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        return SkCornerPathEffect::Make(buffer.readScalar());
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fRadius);
    }

    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkCornerPathEffect"; }

private:
    const SkScalar fRadius;

    using INHERITED = SkPathEffectBase;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkCornerPathEffect::Make(SkScalar radius) {
    return SkScalarIsFinite(radius) && (radius > 0) ?
            sk_sp<SkPathEffect>(new SkCornerPathEffectImpl(radius)) : nullptr;
}

void SkCornerPathEffect::RegisterFlattenables() {
    SkFlattenable::Register("SkCornerPathEffect", SkCornerPathEffectImpl::CreateProc);
}
