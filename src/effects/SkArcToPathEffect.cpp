/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArcToPathEffect.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkArcToPathEffect::SkArcToPathEffect(SkScalar radius) : fRadius(radius) {}

bool SkArcToPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                   SkStrokeRec*, const SkRect*) const {
    SkPath::Iter    iter(src, false);
    SkPath::Verb    verb;
    SkPoint         pts[4];

    SkPoint         lastCorner = { 0, 0 }; // avoid warning
    SkPath::Verb    prevVerb = SkPath::kMove_Verb;

    for (;;) {
        switch (verb = iter.next(pts, false)) {
            case SkPath::kMove_Verb:
                if (SkPath::kLine_Verb == prevVerb) {
                    dst->lineTo(lastCorner);
                }
                dst->moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                if (prevVerb == SkPath::kLine_Verb) {
                    dst->arcTo(pts[0], pts[1], fRadius);
                }
                lastCorner = pts[1];
                break;
            case SkPath::kQuad_Verb:
                dst->quadTo(pts[1], pts[2]);
                lastCorner = pts[2];
                break;
            case SkPath::kConic_Verb:
                dst->conicTo(pts[1], pts[2], iter.conicWeight());
                lastCorner = pts[2];
                break;
            case SkPath::kCubic_Verb:
                dst->cubicTo(pts[1], pts[2], pts[3]);
                lastCorner = pts[3];
                break;
            case SkPath::kClose_Verb:
                dst->lineTo(lastCorner);
                break;
            case SkPath::kDone_Verb:
                dst->lineTo(lastCorner);
                goto DONE;
        }
        prevVerb = verb;
    }
DONE:
    return true;
}

SkFlattenable* SkArcToPathEffect::CreateProc(SkReadBuffer& buffer) {
    return SkArcToPathEffect::Create(buffer.readScalar());
}

void SkArcToPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fRadius);
}

#ifndef SK_IGNORE_TO_STRING
void SkArcToPathEffect::toString(SkString* str) const {
    str->appendf("SkArcToPathEffect: (");
    str->appendf("radius: %f", fRadius);
    str->appendf(")");
}
#endif
