#include "SkDebuggerViews.h"
#include "SkRect.h"

SkInfoPanelView::SkInfoPanelView() {
    fBGColor = 0xFF999999;
    fPaint.setColor(fBGColor);
}

bool SkInfoPanelView::onEvent(const SkEvent& evt) {
    if (evt.isType(SkDebugger_StateType)) {
        fMatrix = evt.findString(SkDebugger_Matrix);
        fClip = evt.findString(SkDebugger_Clip);
        
        SkPaint* ptr;
        if (evt.getMetaData().findPtr(SkDebugger_Paint, (void**)&ptr)) {
            fPaint = *ptr;
            fPaintInfo = evt.findString(SkDebugger_PaintInfo);
        }
        this->inval(NULL);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void SkInfoPanelView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
    
    //Display Current Paint
    SkRect r = {10, 20, 40, 50};
    canvas->drawRect(r, fPaint);
    //Display Information
    SkPaint p;
    p.setTextSize(SkDebugger_TextSize);
    p.setAntiAlias(true);
    int x = 50;
    canvas->drawText(fPaintInfo.c_str(), fPaintInfo.size(), x, 30, p);
    canvas->drawText(fMatrix.c_str(), fMatrix.size(), x, 60, p);
    canvas->drawText(fClip.c_str(), fClip.size(), x, 90, p);
    
    this->INHERITED::onDraw(canvas);
}