/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrokeRec.h"
#include "SkPaintDefaults.h"

// must be < 0, since ==0 means hairline, and >0 means normal stroke
#define kStrokeRec_FillStyleWidth     (-SK_Scalar1)

SkStrokeRec::SkStrokeRec(InitStyle s) {
    fWidth          = (kFill_InitStyle == s) ? kStrokeRec_FillStyleWidth : 0;
    fMiterLimit     = SkPaintDefaults_MiterLimit;
    fCap            = SkPaint::kDefault_Cap;
    fJoin           = SkPaint::kDefault_Join;
    fStrokeAndFill  = false;
}

SkStrokeRec::SkStrokeRec(const SkStrokeRec& src) {
    memcpy(this, &src, sizeof(src));
}

SkStrokeRec::SkStrokeRec(const SkPaint& paint) {
    switch (paint.getStyle()) {
        case SkPaint::kFill_Style:
            fWidth = kStrokeRec_FillStyleWidth;
            fStrokeAndFill = false;
            break;
        case SkPaint::kStroke_Style:
            fWidth = paint.getStrokeWidth();
            fStrokeAndFill = false;
            break;
        case SkPaint::kStrokeAndFill_Style:
            if (0 == paint.getStrokeWidth()) {
                // hairline+fill == fill
                fWidth = kStrokeRec_FillStyleWidth;
                fStrokeAndFill = false;
            } else {
                fWidth = paint.getStrokeWidth();
                fStrokeAndFill = true;
            }
            break;
        default:
            SkASSERT(!"unknown paint style");
            // fall back on just fill
            fWidth = kStrokeRec_FillStyleWidth;
            fStrokeAndFill = false;
            break;
    }

    // copy these from the paint, regardless of our "style"
    fMiterLimit = paint.getStrokeMiter();
    fCap        = paint.getStrokeCap();
    fJoin       = paint.getStrokeJoin();
}

SkStrokeRec::Style SkStrokeRec::getStyle() const {
    if (fWidth < 0) {
        return kFill_Style;
    } else if (0 == fWidth) {
        return kHairline_Style;
    } else {
        return fStrokeAndFill ? kStrokeAndFill_Style : kStroke_Style;
    }
}

void SkStrokeRec::setFillStyle() {
    fWidth = kStrokeRec_FillStyleWidth;
    fStrokeAndFill = false;
}

void SkStrokeRec::setHairlineStyle() {
    fWidth = 0;
    fStrokeAndFill = false;
}

void SkStrokeRec::setStrokeStyle(SkScalar width, bool strokeAndFill) {
    if (strokeAndFill && (0 == width)) {
        // hairline+fill == fill
        this->setFillStyle();
    } else {
        fWidth = width;
        fStrokeAndFill = strokeAndFill;
    }
}

#include "SkStroke.h"

bool SkStrokeRec::applyToPath(SkPath* dst, const SkPath& src) const {
    if (fWidth <= 0) {  // hairline or fill
        return false;
    }

    SkStroke stroker;
    stroker.setCap(fCap);
    stroker.setJoin(fJoin);
    stroker.setMiterLimit(fMiterLimit);
    stroker.setWidth(fWidth);
    stroker.setDoFill(fStrokeAndFill);
    stroker.strokePath(src, dst);
    return true;
}
