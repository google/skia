
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkProgressBarView.h"
#include "SkAnimator.h"
#include "SkWidgetViews.h"
#include "SkTime.h"
#include "SkSystemEventTypes.h"

SkProgressBarView::SkProgressBarView()
{
    init_skin_anim(kProgress_SkinEnum, &fAnim);
    fAnim.setHostEventSink(this);
    fProgress = 0;
    fMax = 100;

}

void SkProgressBarView::changeProgress(int diff)
{
    int newProg = fProgress + diff;
    if (newProg > 0 && newProg < fMax)
        this->setProgress(newProg);
    //otherwise i'll just leave it as it is
    //this implies that if a new max and progress are set, max must be set first
}

/*virtual*/ void SkProgressBarView::onDraw(SkCanvas* canvas)
{
    SkPaint                        paint;
    SkAnimator::DifferenceType    diff = fAnim.draw(canvas, &paint, SkTime::GetMSecs());

    if (diff == SkAnimator::kDifferent)
        this->inval(NULL);
    else if (diff == SkAnimator::kPartiallyDifferent)
    {
        SkRect    bounds;
        fAnim.getInvalBounds(&bounds);
        this->inval(&bounds);
    }
}

/*virtual*/ bool SkProgressBarView::onEvent(const SkEvent& evt)
{
    if (evt.isType(SK_EventType_Inval))
    {
        this->inval(NULL);
        return true;
    }
    if (evt.isType("recommendDim"))
    {
        SkScalar    height;

        if (evt.findScalar("y", &height))
            this->setHeight(height);
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

/*virtual*/ void SkProgressBarView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);
    int32_t temp;
    if (dom.findS32(node, "max", &temp))
        this->setMax(temp);
    if (dom.findS32(node, "progress", &temp))
        this->setProgress(temp);
}

/*virtual*/ void SkProgressBarView::onSizeChange()
{
    this->INHERITED::onSizeChange();
    SkEvent evt("user");
    evt.setString("id", "setDim");
    evt.setScalar("dimX", this->width());
    evt.setScalar("dimY", this->height());
    fAnim.doUserEvent(evt);
}

void SkProgressBarView::reset()
{
    fProgress = 0;
    SkEvent e("user");
    e.setString("id", "reset");
    fAnim.doUserEvent(e);
}

void SkProgressBarView::setMax(int max)
{
    fMax = max;
    SkEvent e("user");
    e.setString("id", "setMax");
    e.setS32("newMax", max);
    fAnim.doUserEvent(e);
}

void SkProgressBarView::setProgress(int progress)
{
    fProgress = progress;
    SkEvent e("user");
    e.setString("id", "setProgress");
    e.setS32("newProgress", progress);
    fAnim.doUserEvent(e);
}
