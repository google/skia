
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DebuggerViews.h"
#include "SkRect.h"

DebuggerStateView::DebuggerStateView() {
    fBGColor = 0xFF999999;
    fPaint.setColor(fBGColor);
    fResizing = false;
}

bool DebuggerStateView::onEvent(const SkEvent& evt) {
    if (evt.isType(SKDEBUGGER_STATETYPE)) {
        fMatrix = evt.findString(SKDEBUGGER_MATRIX);
        fClip = evt.findString(SKDEBUGGER_CLIP);

        SkPaint* ptr;
        if (evt.getMetaData().findPtr(SKDEBUGGER_PAINT, (void**)&ptr)) {
            fPaint = *ptr;
            fPaintInfo = evt.findString(SKDEBUGGER_PAINTINFO);
        }
        this->inval(NULL);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void DebuggerStateView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);

    //Display Current Paint
    SkRect r = {10, 20, 40, 50};
    canvas->drawRect(r, fPaint);
    //Display Information
    SkPaint p;
    p.setTextSize(SKDEBUGGER_TEXTSIZE);
    p.setAntiAlias(true);
    SkScalar x = 50 * SK_Scalar1;
    canvas->drawText(fPaintInfo.c_str(), fPaintInfo.size(), x, 30, p);
    canvas->drawText(fMatrix.c_str(), fMatrix.size(), x, 60, p);
    canvas->drawText(fClip.c_str(), fClip.size(), x, 90, p);

    p.setColor(SKDEBUGGER_RESIZEBARCOLOR);
    r = SkRect::MakeXYWH(0, 0, this->width(), SKDEBUGGER_RESIZEBARSIZE);
    canvas->drawRect(r, p);
}

