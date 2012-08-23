
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDebugDumper.h"
#include "SkString.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkXfermode.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "DebuggerViews.h"

SkDebugDumper::SkDebugDumper(SkEventSinkID cID, SkEventSinkID clID,
                             SkEventSinkID ipID) {
    fContentID = cID;
    fCommandsID = clID;
    fStateID = ipID;
    fInit = false;
    fDisabled = false;
    fCount = 0;
}

static void appendPtr(SkString* str, const void* ptr, const char name[]) {
    if (ptr) {
        str->appendf("%s: %p\t", name, ptr);
    }
}

static void appendFlattenable(SkString* str, const SkFlattenable* ptr,
                              const char name[]) {
    if (ptr) {
        str->appendf("%s: %p\n", name, ptr);
    }
}

static SkString dumpMatrix(SkDumpCanvas* canvas) {
    SkString str;
    SkMatrix m = canvas->getTotalMatrix();
    str.append("Matrix:");
    str.appendf("Translate (%0.4g, %0.4g) ",
                 SkScalarToFloat(m.get(SkMatrix::kMTransX)),
                 SkScalarToFloat(m.get(SkMatrix::kMTransY)));
    str.appendf("Scale (%0.4g, %0.4g) ",
                 SkScalarToFloat(m.get(SkMatrix::kMScaleX)),
                 SkScalarToFloat(m.get(SkMatrix::kMScaleY)));
    str.appendf("Skew (%0.4g, %0.4g) ",
                 SkScalarToFloat(m.get(SkMatrix::kMSkewX)),
                 SkScalarToFloat(m.get(SkMatrix::kMSkewY)));
    str.appendf("Perspective (%0.4g, %0.4g, %0.4g) ",
                 SkScalarToFloat(SkPerspToScalar(m.get(SkMatrix::kMPersp0))),
                 SkScalarToFloat(SkPerspToScalar(m.get(SkMatrix::kMPersp1))),
                 SkScalarToFloat(SkPerspToScalar(m.get(SkMatrix::kMPersp2))));
    return str;
}


static const int maxPts = 50;
static SkString dumpClip(SkDumpCanvas* canvas) {
    SkString str;
    SkPath p;
    if (canvas->getTotalClip().getBoundaryPath(&p)) {
        SkPoint pts[maxPts];
        int numPts = p.getPoints(pts, maxPts);

        str.append("Clip: [ ");
        for (int i = 0; i < numPts; ++i) {
            str.appendf("(%0.4g, %0.4g)", pts[i].x(), pts[i].y());
            if (i < numPts-1)
                str.append(" , ");
        }
        str.append(" ]");
    }
    return str;
}

static const char* gPaintFlags[] = {
    "AntiAliasing",
    "Bitmap Filtering",
    "Dithering",
    "Underline Text",
    "Strike-Through Text",
    "Fake Bold Text",
    "Linear Text",
    "Subpixel Positioned Text",
    "Device Kerning Text",
    "LCD/Subpixel Glyph Rendering",
    "Embedded Bitmap Text",
    "Freetype Autohinting",
    "ALL"
};


static SkString dumpPaint(SkDumpCanvas* canvas, const SkPaint* p,
                      SkDumpCanvas::Verb verb) {
    SkString str;
    str.appendf("Color: #%08X\n", p->getColor());
    str.appendf("Flags: %s\n", gPaintFlags[p->getFlags()]);
    appendFlattenable(&str, p->getShader(), "shader");
    appendFlattenable(&str, p->getXfermode(), "xfermode");
    appendFlattenable(&str, p->getPathEffect(), "pathEffect");
    appendFlattenable(&str, p->getMaskFilter(), "maskFilter");
    appendFlattenable(&str, p->getPathEffect(), "pathEffect");
    appendFlattenable(&str, p->getColorFilter(), "filter");

    if (SkDumpCanvas::kDrawText_Verb == verb) {
        str.appendf("Text Size:%0.4g\n", SkScalarToFloat(p->getTextSize()));
        appendPtr(&str, p->getTypeface(), "typeface");
    }

    return str;
}

void SkDebugDumper::dump(SkDumpCanvas* canvas, SkDumpCanvas::Verb verb,
                          const char str[], const SkPaint* p) {
    if (!fDisabled) {
        SkString msg, tab;

        const int level = canvas->getNestLevel() + canvas->getSaveCount() - 1;
        SkASSERT(level >= 0);
        for (int i = 0; i < level; i++) {
            tab.append("| ");
        }

        msg.appendf("%03d: %s%s\n", fCount, tab.c_str(), str);
        ++fCount;
        if (!fInit) {
            SkEvent* cmd = new SkEvent(SKDEBUGGER_COMMANDTYPE, fCommandsID);
            cmd->setString(SKDEBUGGER_ATOM, msg);
            cmd->postDelay(100);
        }
        else {
            SkEvent* state = new SkEvent(SKDEBUGGER_STATETYPE, fStateID);
            state->setString(SKDEBUGGER_MATRIX, dumpMatrix(canvas));
            state->setString(SKDEBUGGER_CLIP, dumpClip(canvas));
            if (p) {
                state->setString(SKDEBUGGER_PAINTINFO, dumpPaint(canvas, p, verb));
                state->getMetaData().setPtr(SKDEBUGGER_PAINT, (void*)p, PaintProc);
            }
            state->post();
        }
    }
}
