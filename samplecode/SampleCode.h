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
#include "SkKey.h"
#include "SkView.h"
#include "SkOSMenu.h"

class GrContext;
class SkAnimTimer;

#define DEF_SAMPLE(code) \
    static SkView*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SkViewRegister   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

#define MAX_ZOOM_LEVEL  8
#define MIN_ZOOM_LEVEL  -8

static const char gCharEvtName[] = "SampleCode_Char_Event";
static const char gKeyEvtName[] = "SampleCode_Key_Event";
static const char gTitleEvtName[] = "SampleCode_Title_Event";
static const char gPrefSizeEvtName[] = "SampleCode_PrefSize_Event";
static const char gFastTextEvtName[] = "SampleCode_FastText_Event";
static const char gUpdateWindowTitleEvtName[] = "SampleCode_UpdateWindowTitle";

class SampleCode {
public:
    static bool KeyQ(const SkEvent&, SkKey* outKey);
    static bool CharQ(const SkEvent&, SkUnichar* outUni);

    static bool TitleQ(const SkEvent&);
    static void TitleR(SkEvent*, const char title[]);
    static bool RequestTitle(SkView* view, SkString* title);

    static bool PrefSizeQ(const SkEvent&);
    static void PrefSizeR(SkEvent*, SkScalar width, SkScalar height);

    static bool FastTextQ(const SkEvent&);

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

namespace skiagm {
class GM;
}

// factory function that creates a skiagm::GM
typedef skiagm::GM* (*GMFactoryFunc)(void*);

// Takes a GM factory function and implements the SkViewFactory interface
// by making the GM and wrapping it in a GMSampleView. GMSampleView bridges
// the SampleView interface to skiagm::GM.
class SkGMSampleViewFactory : public SkViewFactory {
public:
    SkGMSampleViewFactory(GMFactoryFunc func);
    SkView* operator() () const override;
private:
    GMFactoryFunc fFunc;
};

class SkViewRegister : public SkRefCnt {
public:
    explicit SkViewRegister(SkViewFactory*);
    explicit SkViewRegister(SkViewCreateFunc);
    explicit SkViewRegister(GMFactoryFunc);

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
        : fPipeState(SkOSMenu::kOffState)
        , fBGColor(SK_ColorWHITE)
        , fRepeatCount(1)
        , fHaveCalledOnceBeforeDraw(false)
    {}

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(const SkAnimTimer& timer) { return this->onAnimate(timer); }

    static bool IsSampleView(SkView*);
    static bool SetRepeatDraw(SkView*, int count);
    static bool SetUsePipe(SkView*, SkOSMenu::TriState);

    /**
     *  Call this to request menu items from a SampleView.
     *  Subclassing notes: A subclass of SampleView can overwrite this method
     *  to add new items of various types to the menu and change its title.
     *  The events attached to any new menu items must be handled in its onEvent
     *  method. See SkOSMenu.h for helper functions.
     */
    virtual void requestMenu(SkOSMenu* menu) {}

    virtual void onTileSizeChanged(const SkSize& tileSize) {}

protected:
    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(const SkAnimTimer&) { return false; }
    virtual void onOnceBeforeDraw() {}

    // overrides
    virtual bool onEvent(const SkEvent& evt);
    virtual bool onQuery(SkEvent* evt);
    virtual void onDraw(SkCanvas*);

    SkOSMenu::TriState fPipeState;
    SkColor fBGColor;

private:
    int fRepeatCount;
    bool fHaveCalledOnceBeforeDraw;
    typedef SkView INHERITED;
};

#endif
