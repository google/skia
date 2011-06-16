#include "SkView.h"
#include "SkColor.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGPipe.h"
#include "SkPaint.h"

#include "SkDebugDumper.h"
#include <deque> 
#define SkDebugger_TextSize 14

#define SkDebugger_CommandType  "SkDebugger_Command"
#define SkDebugger_StateType    "SkDebugger_State"

#define SkDebugger_Atom         "SkDebugger_Atom"
#define SkDebugger_Matrix       "SkDebugger_Matrix"
#define SkDebugger_Clip         "SkDebugger_Clip"
#define SkDebugger_PaintInfo    "SkDebugger_PaintInfo"
#define SkDebugger_Paint        "SkDebugger_Paint"

/*
 * Debugger - Main Content
 */
class SkContentView : public SkView {
public:
    SkContentView(SkEventSinkID clID, SkEventSinkID ipID);
    ~SkContentView();
    
    void init();
    void reinit(const char* fileName);
    void toggleClip();
    void goToAtom(int atom);
    
protected:
    virtual bool onEvent(const SkEvent& evt);
    virtual void onDraw(SkCanvas* canvas);
    
private:
    SkColor         fBGColor;
    int             fAtomsToRead;
    std::deque<int> fAtomBounds;
    std::deque<int> fFrameBounds;
    bool            fDisplayClip;
    SkString        fFilePath;
    SkDebugDumper   fDumper;
    typedef SkView INHERITED;
};

/*
 * Debugger - Info Panel
 */
class SkInfoPanelView : public SkView {
public:
    SkInfoPanelView();
    
protected:
    virtual bool onEvent(const SkEvent& evt);
    virtual void onDraw(SkCanvas* canvas);
    
private:
    SkColor     fBGColor;
    SkPaint     fPaint;
    SkString    fMatrix;
    SkString    fPaintInfo;
    SkString    fClip;
    typedef SkView INHERITED;
};

/*
 * Debugger - Commands List
 */
class SkCommandListView : public SkView {
public:
    SkCommandListView();
    void reinit();
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
    void init();
    void alignCenter();
    SkColor     fBGColor;
    int         fTopIndex;
    int         fHighlight;
    SkScalar    fSpacing;
    int         fRange;
    bool        fCentered;
    std::deque<SkString> fList;
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

