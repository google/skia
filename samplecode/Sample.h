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
#include "tools/Registry.h"
#include "tools/SkMetaData.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"

class SkCanvas;
class Sample;

using SampleFactory = Sample* (*)();
using SampleRegistry = sk_tools::Registry<SampleFactory>;

#define DEF_SAMPLE(code) \
    static Sample*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SampleRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

///////////////////////////////////////////////////////////////////////////////

class Sample {
public:
    Sample()
        : fBGColor(SK_ColorWHITE)
        , fWidth(0), fHeight(0)
        , fHaveCalledOnceBeforeDraw(false)
    {}

    virtual ~Sample() = default;

    SkScalar    width() const { return fWidth; }
    SkScalar    height() const { return fHeight; }
    void        setSize(SkScalar width, SkScalar height);
    void        setSize(const SkPoint& size) { this->setSize(size.fX, size.fY); }
    void        setWidth(SkScalar width) { this->setSize(width, fHeight); }
    void        setHeight(SkScalar height) { this->setSize(fWidth, height); }

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    virtual bool onChar(SkUnichar) { return false; }

    // Click handling
    class Click {
    public:
        virtual ~Click() = default;
        SkPoint     fOrig = {0, 0};
        SkPoint     fPrev = {0, 0};
        SkPoint     fCurr = {0, 0};
        skui::InputState  fState = skui::InputState::kDown;
        skui::ModifierKey fModifierKeys = skui::ModifierKey::kNone;
        SkMetaData  fMeta;
    };
    bool mouse(SkPoint point, skui::InputState clickState, skui::ModifierKey modifierKeys);

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(double nanos) { return this->onAnimate(nanos); }

    virtual SkString name() = 0;

protected:
    /** Override to be notified of size changes. Overriders must call the super class. */
    virtual void onSizeChange();

    /** Override this if you might handle the click */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi);

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*);

    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(double /*nanos*/) { return false; }
    virtual void onOnceBeforeDraw() {}

private:
    std::unique_ptr<Click> fClick;
    SkColor fBGColor;
    SkScalar fWidth, fHeight;
    bool fHaveCalledOnceBeforeDraw;

    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;
};

#endif
