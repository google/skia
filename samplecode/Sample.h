/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "gm/gm.h"
#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkMacros.h"
#include "tools/InputState.h"
#include "tools/ModifierKey.h"
#include "tools/Registry.h"
#include "tools/SkMetaData.h"

class SkCanvas;
class Sample;

using SampleFactory = Sample* (*)();
using SampleRegistry = sk_tools::Registry<SampleFactory>;

#define DEF_SAMPLE(code) \
    static Sample*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SampleRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

///////////////////////////////////////////////////////////////////////////////

class Sample : public skiagm::GM {
public:
    SkSize windowSize() const { return fWindowSize; }
    SkScalar width() const { return fWindowSize.width(); }
    SkScalar height() const { return fWindowSize.height(); }

    void setWindowSize(SkSize);
    void setSize(SkScalar w, SkScalar h) { this->setWindowSize({w, h}); }

    /** Call this to have the view draw into the specified canvas. */
    void drawSample(SkCanvas* canvas);

    bool mouse(SkPoint point, InputState clickState, ModifierKey modifierKeys);

    // Click handling
    class Click {
    public:
        virtual ~Click() = default;
        SkPoint     fOrig = {0, 0};
        SkPoint     fPrev = {0, 0};
        SkPoint     fCurr = {0, 0};
        InputState  fState = InputState::kDown;
        ModifierKey fModifierKeys = ModifierKey::kNone;
        SkMetaData  fMeta;
    };

protected:
    Sample() = default;

    /** Override to be notified of size changes. Overriders must call the super class. */
    virtual void onSizeChange();

    /** Override this if you might handle the click */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi);

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*);

    SkISize onISize() override { return {512, 512}; }

private:
    std::unique_ptr<Click> fClick;
    SkSize fWindowSize = {0, 0};

    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;
};

#endif
