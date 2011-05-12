#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "SkColor.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkView.h"

class SampleCode {
public:
    static bool KeyQ(const SkEvent&, SkKey* outKey);
    static bool CharQ(const SkEvent&, SkUnichar* outUni);

    static bool TitleQ(const SkEvent&);
    static void TitleR(SkEvent*, const char title[]);
    
    static bool PrefSizeQ(const SkEvent&);
    static void PrefSizeR(SkEvent*, SkScalar width, SkScalar height);

    static bool FastTextQ(const SkEvent&);

    static SkMSec GetAnimTime();
    static SkMSec GetAnimTimeDelta();
    static SkScalar GetAnimSecondsDelta();
    static SkScalar GetAnimScalar(SkScalar speedPerSec, SkScalar period = 0);
};

//////////////////////////////////////////////////////////////////////////////

typedef SkView* (*SkViewFactory)();

class SkViewRegister : SkNoncopyable {
public:
    explicit SkViewRegister(SkViewFactory);
    
    static const SkViewRegister* Head() { return gHead; }
    
    SkViewRegister* next() const { return fChain; }
    SkViewFactory   factory() const { return fFact; }
    
private:
    SkViewFactory   fFact;
    SkViewRegister* fChain;
    
    static SkViewRegister* gHead;
};

///////////////////////////////////////////////////////////////////////////////

class SampleView : public SkView {
public:
    SampleView() : fRepeatCount(1), fBGColor(SK_ColorWHITE) {
        fUsePipe = false;
    }

    void setBGColor(SkColor color) { fBGColor = color; }

    static bool IsSampleView(SkView*);
    static bool SetRepeatDraw(SkView*, int count);
    static bool SetUsePipe(SkView*, bool);

protected:
    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;

    // overrides
    virtual bool onEvent(const SkEvent& evt);
    virtual bool onQuery(SkEvent* evt);
    virtual void onDraw(SkCanvas*);

private:
    int fRepeatCount;
    SkColor fBGColor;

    bool fUsePipe;

    typedef SkView INHERITED;
};

#endif

