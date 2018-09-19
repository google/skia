/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrokeRec.h"
#include "src/core/SkPaintDefaults.h"

// must be < 0, since ==0 means hairline, and >0 means normal stroke
#define kStrokeRec_FillStyleWidth     (-SK_Scalar1)

SkStrokeRec::SkStrokeRec(InitStyle s) {
    fResScale       = 1;
    fWidth          = (kFill_InitStyle == s) ? kStrokeRec_FillStyleWidth : 0;
    fMiterLimit     = SkPaintDefaults_MiterLimit;
    fCap            = SkPaint::kDefault_Cap;
    fJoin           = SkPaint::kDefault_Join;
    fStrokeAndFill  = false;
}

SkStrokeRec::SkStrokeRec(const SkPaint& paint, SkScalar resScale) {
    this->init(paint, paint.getStyle(), resScale);
}

SkStrokeRec::SkStrokeRec(const SkPaint& paint, SkPaint::Style styleOverride, SkScalar resScale) {
    this->init(paint, styleOverride, resScale);
}

void SkStrokeRec::init(const SkPaint& paint, SkPaint::Style style, SkScalar resScale) {
    fResScale = resScale;

    switch (style) {
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
            SkDEBUGFAIL("unknown paint style");
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

#include "src/core/SkStroke.h"

#ifdef SK_DEBUG
    // enables tweaking these values at runtime from Viewer
    bool gDebugStrokerErrorSet = false;
    SkScalar gDebugStrokerError;
#endif

bool SkStrokeRec::applyToPath(SkPath* dst, const SkPath& src) const {
    if (fWidth <= 0) {  // hairline or fill
        return false;
    }

    SkStroke stroker;
    stroker.setCap((SkPaint::Cap)fCap);
    stroker.setJoin((SkPaint::Join)fJoin);
    stroker.setMiterLimit(fMiterLimit);
    stroker.setWidth(fWidth);
    stroker.setDoFill(fStrokeAndFill);
#ifdef SK_DEBUG
    stroker.setResScale(gDebugStrokerErrorSet ? gDebugStrokerError : fResScale);
#else
    stroker.setResScale(fResScale);
#endif
    stroker.strokePath(src, dst);
    return true;
}

void SkStrokeRec::applyToPaint(SkPaint* paint) const {
    if (fWidth < 0) {  // fill
        paint->setStyle(SkPaint::kFill_Style);
        return;
    }

    paint->setStyle(fStrokeAndFill ? SkPaint::kStrokeAndFill_Style : SkPaint::kStroke_Style);
    paint->setStrokeWidth(fWidth);
    paint->setStrokeMiter(fMiterLimit);
    paint->setStrokeCap((SkPaint::Cap)fCap);
    paint->setStrokeJoin((SkPaint::Join)fJoin);
}

SkScalar SkStrokeRec::getInflationRadius() const {
    return GetInflationRadius((SkPaint::Join)fJoin, fMiterLimit, (SkPaint::Cap)fCap, fWidth);
}

SkScalar SkStrokeRec::GetInflationRadius(const SkPaint& paint, SkPaint::Style style) {
    SkScalar width = SkPaint::kFill_Style == style ? -SK_Scalar1 : paint.getStrokeWidth();
    return GetInflationRadius(paint.getStrokeJoin(), paint.getStrokeMiter(), paint.getStrokeCap(),
                              width);

}

SkScalar SkStrokeRec::GetInflationRadius(SkPaint::Join join, SkScalar miterLimit, SkPaint::Cap cap,
                                         SkScalar strokeWidth) {
    if (strokeWidth < 0) {  // fill
        return 0;
    } else if (0 == strokeWidth) {
        // FIXME: We need a "matrixScale" parameter here in order to properly handle hairlines.
        // Their with is determined in device space, unlike other strokes.
        // http://skbug.com/8157
        return SK_Scalar1;
    }

    // since we're stroked, outset the rect by the radius (and join type, caps)
    SkScalar multiplier = SK_Scalar1;
    if (SkPaint::kMiter_Join == join) {
        multiplier = SkTMax(multiplier, miterLimit);
    }
    if (SkPaint::kSquare_Cap == cap) {
        multiplier = SkTMax(multiplier, SK_ScalarSqrt2);
    }
    return strokeWidth/2 * multiplier;
}

