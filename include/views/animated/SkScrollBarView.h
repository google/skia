
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkScrollBarView_DEFINED
#define SkScrollBarView_DEFINED

#include "SkView.h"
#include "SkWidgetViews.h"
#include "SkAnimator.h"

class SkScrollBarView : public SkWidgetView {
public:
    SkScrollBarView();

    unsigned getStart() const { return fStartPoint; }
    unsigned getShown() const { return fShownLength; }
    unsigned getTotal() const { return fTotalLength; }

    void setStart(unsigned start);
    void setShown(unsigned shown);
    void setTotal(unsigned total);

protected:
    //overrides
    virtual void onInflate(const SkDOM& dom, const SkDOM::Node* node);
    virtual void onSizeChange();
    virtual void onDraw(SkCanvas* canvas);
    virtual bool onEvent(const SkEvent& evt);

private:
    SkAnimator  fAnim;
    unsigned    fTotalLength, fStartPoint, fShownLength;

    void adjust();

    typedef SkWidgetView INHERITED;
};
#endif

