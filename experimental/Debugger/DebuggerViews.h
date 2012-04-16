
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkView.h"
#include "SkColor.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGPipe.h"
#include "SkPaint.h"

#include "SkDebugDumper.h"

#define SKDEBUGGER_COMMANDTYPE  "SKDEBUGGER_COMMAND"
#define SKDEBUGGER_STATETYPE    "SKDEBUGGER_STATE"

#define SKDEBUGGER_ATOM         "SKDEBUGGER_ATOM"
#define SKDEBUGGER_MATRIX       "SKDEBUGGER_MATRIX"
#define SKDEBUGGER_CLIP         "SKDEBUGGER_CLIP"
#define SKDEBUGGER_PAINTINFO    "SKDEBUGGER_PAINTINFO"
#define SKDEBUGGER_PAINT        "SKDEBUGGER_PAINT"

#define SKDEBUGGER_TEXTSIZE         14
#define CMD_WIDTH                   200
#define INFO_HEIGHT                 150.0f
#define SKDEBUGGER_HIGHLIGHTCOLOR   0xFF113399
#define SKDEBUGGER_TEXTCOLOR        0xFF000000
#define SKDEBUGGER_RESIZEBARCOLOR   0xFF333333
#define SKDEBUGGER_RESIZEBARSIZE    5

/*
 * Debugger - Info Panel
 */
class DebuggerStateView : public SkView {
public:
    DebuggerStateView();
    
protected:
    virtual bool onEvent(const SkEvent& evt);
    virtual void onDraw(SkCanvas* canvas);
private:
    SkColor     fBGColor;
    SkPaint     fPaint;
    SkString    fMatrix;
    SkString    fPaintInfo;
    SkString    fClip;
    bool        fResizing;
    typedef SkView INHERITED;
};

/*
 * Debugger - Commands List
 */
class DebuggerCommandsView : public SkView {
public:
    DebuggerCommandsView();
    ~DebuggerCommandsView();
    int nextItem();
    int prevItem();
    int scrollUp();
    int scrollDown();
    void highlight(int index);
    int  selectHighlight(int ypos);
    void toggleCentered();
    
protected:
    virtual bool onEvent(const SkEvent& evt);
    virtual void onSizeChange();
    virtual void onDraw(SkCanvas* canvas);
private:
    void        init();
    void        alignCenter();
    SkColor     fBGColor;
    int         fTopIndex;
    int         fHighlight;
    SkScalar    fSpacing;
    int         fRange;
    bool        fResizing;
    bool        fCentered;
    SkTDArray<SkString*> fList;
    typedef SkView INHERITED;
};


static void* PaintProc(void* ptr, bool doRef) {
    SkPaint* p = (SkPaint*) ptr;
    
    if (doRef) {
        return new SkPaint(*p);
    }
    else {
        delete p;
        return NULL;
    }
    
}

