
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DebuggerViews.h"

DebuggerCommandsView::DebuggerCommandsView() {
    fBGColor = 0xFFBBBBBB;
    fTopIndex = 0;
    fHighlight = 0;
    fResizing = false;

    SkPaint p;
    p.setTextSize(SkIntToScalar(SKDEBUGGER_TEXTSIZE));
    fSpacing = p.getFontSpacing();
    fCentered = false;
    fRange = (int)(this->height()/fSpacing) - 1;
}

DebuggerCommandsView::~DebuggerCommandsView() {
    fList.deleteAll();
}

bool DebuggerCommandsView::onEvent(const SkEvent& evt) {
    if (evt.isType(SKDEBUGGER_COMMANDTYPE)) {
        *fList.append() = new SkString(evt.findString(SKDEBUGGER_ATOM));
        this->inval(NULL);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void DebuggerCommandsView::onSizeChange() {
    fRange = (int)(this->height()/fSpacing);
    this->INHERITED::onSizeChange();
}

void DebuggerCommandsView::alignCenter() {
    if (!fCentered || fHighlight < fRange/2 || fHighlight > (fList.count() - fRange/2)) {
        return;
    } else {
        if (fHighlight > (fTopIndex + fRange/2))
            fTopIndex += fHighlight - (fTopIndex + fRange/2);
        if (fHighlight < (fTopIndex + fRange/2))
            fTopIndex -= (fTopIndex + fRange/2) - fHighlight;
    }
}

int DebuggerCommandsView::nextItem() {
    if (fHighlight < fList.count() - 1)
        ++fHighlight;
    if (fHighlight < fTopIndex || fHighlight > (fTopIndex + fRange))
        fTopIndex = fHighlight;
    if (fHighlight == (fTopIndex + fRange))
        ++fTopIndex;
    this->alignCenter();
    this->inval(NULL);
    return fHighlight;
}

int DebuggerCommandsView::prevItem() {
    if (fHighlight > 0)
        --fHighlight;
    if (fHighlight < fTopIndex || fHighlight > (fTopIndex + fRange))
        fTopIndex = fHighlight;
    this->alignCenter();
    this->inval(NULL);
    return fHighlight;
}

int DebuggerCommandsView::scrollUp() {
    if (fTopIndex > 0)
        --fTopIndex;
    this->inval(NULL);
    return fHighlight;
}

int DebuggerCommandsView::scrollDown() {
    if (fTopIndex < (fList.count() - 1))
        ++fTopIndex;
    this->inval(NULL);
    return fHighlight;
}

void DebuggerCommandsView::highlight(int index) {
    SkASSERT(index >= 0 && index < fList.count());
    if (fHighlight != index) {
        fHighlight = index;
        this->alignCenter();
        this->inval(NULL);
    }
}

int DebuggerCommandsView::selectHighlight(int ypos) {
    int i = (int)(ypos/fSpacing) + fTopIndex;
    if (i >= fList.count()) {
        i = fList.count() - 1;
    }
    if (fHighlight != i) {
        fHighlight = i;
        this->alignCenter();
        this->inval(NULL);
    }
    return fHighlight;
}

void DebuggerCommandsView::toggleCentered() {
    fCentered = !fCentered;
    this->alignCenter();
    this->inval(NULL);
}

void DebuggerCommandsView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);

    SkPaint p;
    p.setTextSize(SkIntToScalar(SKDEBUGGER_TEXTSIZE));
    p.setAntiAlias(true);

    //draw highlight
    int selected = fHighlight - fTopIndex;
    SkRect r = {0, fSpacing * selected, this->width(), fSpacing * (selected+1)};
    p.setColor(SKDEBUGGER_HIGHLIGHTCOLOR);
    canvas->drawRect(r, p);

    int endIndex = fTopIndex + fRange;
    if (endIndex > fList.count())
        endIndex = fList.count();

    p.setColor(SKDEBUGGER_TEXTCOLOR);
    int pos;
    for (int i = fTopIndex; i < endIndex; ++i) {
        pos = i - fTopIndex;
        canvas->drawText(fList[i]->c_str(), fList[i]->size(),
                         0, fSpacing - 2 + fSpacing * pos, p);
    }
    p.setColor(SKDEBUGGER_RESIZEBARCOLOR);
    r = SkRect::MakeXYWH(this->width() - SKDEBUGGER_RESIZEBARSIZE, 0,
                         SKDEBUGGER_RESIZEBARSIZE, this->height());
    canvas->drawRect(r, p);
}

