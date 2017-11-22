/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "SkColor.h"
#include "SkEvent.h"
#include "SkView.h"

class SkAnimTimer;

#define DEF_SAMPLE(code) \
    static SkView*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SkViewRegister   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

static const char gCharEvtName[] = "SampleCode_Char_Event";
static const char gTitleEvtName[] = "SampleCode_Title_Event";

class SampleCode {
public:
    static bool CharQ(const SkEvent&, SkUnichar* outUni);

    static bool TitleQ(const SkEvent&);
    static void TitleR(SkEvent*, const char title[]);
    static bool RequestTitle(SkView* view, SkString* title);

    friend class SampleWindow;
};

//////////////////////////////////////////////////////////////////////////////

// interface that constructs SkViews
class SkViewFactory : public SkRefCnt {
public:
    virtual SkView* operator() () const = 0;
};

typedef SkView* (*SkViewCreateFunc)();

// wraps SkViewCreateFunc in SkViewFactory interface
class SkFuncViewFactory : public SkViewFactory {
public:
    SkFuncViewFactory(SkViewCreateFunc func);
    SkView* operator() () const override;

private:
    SkViewCreateFunc fCreateFunc;
};

class SkViewRegister : public SkRefCnt {
public:
    explicit SkViewRegister(SkViewFactory*);
    explicit SkViewRegister(SkViewCreateFunc);

    ~SkViewRegister() {
        fFact->unref();
    }

    static const SkViewRegister* Head() { return gHead; }

    SkViewRegister* next() const { return fChain; }
    const SkViewFactory*   factory() const { return fFact; }

private:
    SkViewFactory*  fFact;
    SkViewRegister* fChain;

    static SkViewRegister* gHead;
};

///////////////////////////////////////////////////////////////////////////////

class SampleView : public SkView {
public:
    SampleView()
        : fBGColor(SK_ColorWHITE)
        , fHaveCalledOnceBeforeDraw(false)
    {}

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(const SkAnimTimer& timer) { return this->onAnimate(timer); }

    static bool IsSampleView(SkView*);

protected:
    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(const SkAnimTimer&) { return false; }
    virtual void onOnceBeforeDraw() {}

    // overrides
    virtual bool onQuery(SkEvent* evt);
    virtual void onDraw(SkCanvas*);

    SkColor fBGColor;

private:
    bool fHaveCalledOnceBeforeDraw;
    typedef SkView INHERITED;
};

#endif
