#include "SkDebuggerViews.h"

SkCommandListView::SkCommandListView() {
    fBGColor = 0xFFBBBBBB;
    fTopIndex = 0;
    fHighlight = 0;
    
    SkPaint p;
    p.setTextSize(SkIntToScalar(SkDebugger_TextSize));
    fSpacing = p.getFontSpacing();
    fCentered = false;
    fRange = (int)(this->height()/fSpacing) - 1;
}

bool SkCommandListView::onEvent(const SkEvent& evt) {
    if (evt.isType(SkDebugger_CommandType)) {
        SkString msg(evt.findString(SkDebugger_Atom));
        fList.push_back(msg);
        this->inval(NULL);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void SkCommandListView::onSizeChange() {
    fRange = (int)(this->height()/fSpacing) - 1;
    this->INHERITED::onSizeChange();
}

void SkCommandListView::reinit() {
    fList.clear();
    fTopIndex = 0;
    fHighlight = 0;
}

void SkCommandListView::alignCenter() {
    if (!fCentered || fHighlight < fRange/2 || fHighlight > (fList.size() - fRange/2))
        return;
    else {
        if (fHighlight > (fTopIndex + fRange/2)) {
            fTopIndex += fHighlight - (fTopIndex + fRange/2);
        }
        if (fHighlight < (fTopIndex + fRange/2)) {
            fTopIndex -= (fTopIndex + fRange/2) - fHighlight;
        }
    }
}

int SkCommandListView::nextItem() {
    if (fHighlight < fList.size() - 1)
        ++fHighlight;
    if (fHighlight < fTopIndex || fHighlight > (fTopIndex + fRange)) {
        fTopIndex = fHighlight;
    }
    if (fHighlight == (fTopIndex + fRange)) {
        ++fTopIndex;
    }
    this->alignCenter();
    this->inval(NULL);
    return fHighlight;
}

int SkCommandListView::prevItem() {
    if (fHighlight > 0)
        --fHighlight;
    if (fHighlight < fTopIndex || fHighlight > (fTopIndex + fRange)) {
        fTopIndex = fHighlight;
    }
    this->alignCenter();
    this->inval(NULL);
    return fHighlight;
}

int SkCommandListView::scrollUp() {
    if (fTopIndex > 0)
        --fTopIndex;
    this->inval(NULL);
    return fHighlight;
}

int SkCommandListView::scrollDown() {
    if (fTopIndex < (fList.size() - 1))
        ++fTopIndex;
    this->inval(NULL);
    return fHighlight;
}

void SkCommandListView::highlight(int index) {
    SkASSERT(index >= 0 && index < fList.size());
    if (fHighlight != index) {
        fHighlight = index;
        this->alignCenter();
        this->inval(NULL);
    }
}

int SkCommandListView::selectHighlight(int ypos) {
    int i = (int)(ypos/fSpacing) + fTopIndex;
    if (i >= fList.size()) {
        i = fList.size() - 1;
    }
    if (fHighlight != i) {
        fHighlight = i;
        this->alignCenter();
        this->inval(NULL);
    }
    return fHighlight;
}

void SkCommandListView::toggleCentered() {
    fCentered = !fCentered;
    this->alignCenter();
    this->inval(NULL);
}

void SkCommandListView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
    
    SkPaint p;
    p.setTextSize(SkIntToScalar(SkDebugger_TextSize));
    p.setAntiAlias(true);
    
    //draw highlight
    int selected = fHighlight - fTopIndex;
    SkRect r = {0, fSpacing * selected, this->width(), fSpacing * (selected+1)};
    p.setColor(0x880033DD);
    canvas->drawRect(r, p);
    
    int endIndex = fTopIndex + fRange; 
    if (endIndex > fList.size())
        endIndex = fList.size();
    
    p.setColor(0xFF000000);
    int pos;
    for (int i = fTopIndex; i < endIndex; ++i) {
        pos = i - fTopIndex;
        canvas->drawText(fList[i].c_str(), fList[i].size(), 
                         0, fSpacing - 2 + fSpacing * pos, p);
    }
    this->INHERITED::onDraw(canvas);
}