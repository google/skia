/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkMacros.h"
#include "src/utils/SkMetaData.h"
#include "tools/Registry.h"

class AnimTimer;
class SkCanvas;
class Sample;

using SampleFactory = Sample* (*)();
using SampleRegistry = sk_tools::Registry<SampleFactory>;

#define DEF_SAMPLE(code) \
    static Sample*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SampleRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

///////////////////////////////////////////////////////////////////////////////

class Sample : public SkRefCnt {
public:
    Sample()
        : fBGColor(SK_ColorWHITE)
        , fWidth(0), fHeight(0)
        , fHaveCalledOnceBeforeDraw(false)
    {}

    SkScalar    width() const { return fWidth; }
    SkScalar    height() const { return fHeight; }
    void        setSize(SkScalar width, SkScalar height);
    void        setSize(const SkPoint& size) { this->setSize(size.fX, size.fY); }
    void        setWidth(SkScalar width) { this->setSize(width, fHeight); }
    void        setHeight(SkScalar height) { this->setSize(fWidth, height); }

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    virtual bool onChar(SkUnichar) { return false; }

    //  Click handling
    class Click {
    public:
        Click(Sample* target);
        virtual ~Click();

        enum State {
            kDown_State,
            kMoved_State,
            kUp_State
        };
        enum ModifierKeys {
            kShift_ModifierKey    = 1 << 0,
            kControl_ModifierKey  = 1 << 1,
            kOption_ModifierKey   = 1 << 2,   // same as ALT
            kCommand_ModifierKey  = 1 << 3,
        };
        SkPoint     fOrig, fPrev, fCurr;
        SkIPoint    fIOrig, fIPrev, fICurr;
        State       fState;
        unsigned    fModifierKeys;

        SkMetaData  fMeta;
    private:
        sk_sp<Sample> fTarget;

        friend class Sample;
    };
    Click* findClickHandler(SkScalar x, SkScalar y, unsigned modifierKeys);
    static void DoClickDown(Click*, int x, int y, unsigned modi);
    static void DoClickMoved(Click*, int x, int y, unsigned modi);
    static void DoClickUp(Click*, int x, int y, unsigned modi);

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(const AnimTimer& timer) { return this->onAnimate(timer); }

    virtual SkString name() = 0;

protected:
    /** Override to be notified of size changes. Overriders must call the super class. */
    virtual void onSizeChange();

    /** Override this if you might handle the click */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi);

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*);

    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(const AnimTimer&) { return false; }
    virtual void onOnceBeforeDraw() {}

private:
    SkColor fBGColor;
    SkScalar fWidth, fHeight;
    bool fHaveCalledOnceBeforeDraw;
};

#endif
