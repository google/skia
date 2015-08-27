
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkScrollBarView.h"
#include "SkAnimator.h"
#include "SkWidgetViews.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"

//see SkProgressBarView.cpp
//#include "SkWidgetViews.cpp"

SkScrollBarView::SkScrollBarView()
{
    fAnim.setHostEventSink(this);
    init_skin_anim(kScroll_SkinEnum, &fAnim);

    fTotalLength = 0;
    fStartPoint = 0;
    fShownLength = 0;

    this->adjust();
}

void SkScrollBarView::setStart(unsigned start)
{
    if ((int)start < 0)
        start = 0;

    if (fStartPoint != start)
    {
        fStartPoint = start;
        this->adjust();
    }
}

void SkScrollBarView::setShown(unsigned shown)
{
    if ((int)shown < 0)
        shown = 0;

    if (fShownLength != shown)
    {
        fShownLength = shown;
        this->adjust();
    }
}

void SkScrollBarView::setTotal(unsigned total)
{
    if ((int)total < 0)
        total = 0;

    if (fTotalLength != total)
    {
        fTotalLength = total;
        this->adjust();
    }
}

/* virtual */ void SkScrollBarView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    int32_t value;
    if (dom.findS32(node, "total", &value))
        this->setTotal(value);
    if (dom.findS32(node, "shown", &value))
        this->setShown(value);
}

/*virtual*/ void SkScrollBarView::onSizeChange()
{
    this->INHERITED::onSizeChange();
    SkEvent evt("user");
    evt.setString("id", "setDim");
    evt.setScalar("dimX", this->width());
    evt.setScalar("dimY", this->height());
    fAnim.doUserEvent(evt);
}

/*virtual*/ void SkScrollBarView::onDraw(SkCanvas* canvas)
{
    SkPaint                        paint;
    SkAnimator::DifferenceType    diff = fAnim.draw(canvas, &paint, SkTime::GetMSecs());

    if (diff == SkAnimator::kDifferent)
        this->inval(nullptr);
    else if (diff == SkAnimator::kPartiallyDifferent)
    {
        SkRect    bounds;
        fAnim.getInvalBounds(&bounds);
        this->inval(&bounds);
    }
}

/*virtual*/ bool SkScrollBarView::onEvent(const SkEvent& evt)
{
    if (evt.isType(SK_EventType_Inval))
    {
        this->inval(nullptr);
        return true;
    }
    if (evt.isType("recommendDim"))
    {
        SkScalar    width;

        if (evt.findScalar("x", &width))
            this->setWidth(width);
        return true;
    }

    return this->INHERITED::onEvent(evt);
}

void SkScrollBarView::adjust()
{
    int total = fTotalLength;
    int start = fStartPoint;
    int shown = fShownLength;
//    int hideBar = 0;

    if (total <= 0 || shown <= 0 || shown >= total)    // no bar to show
    {
        total = 1;        // avoid divide-by-zero. should be done by skin/script
//        hideBar = 1;    // signal we don't want a thumb
    }
    else
    {
        if (start + shown > total)
            start = total - shown;
    }

    SkEvent e("user");
    e.setString("id", "adjustScrollBar");
    e.setScalar("_totalLength", SkIntToScalar(total));
    e.setScalar("_startPoint", SkIntToScalar(start));
    e.setScalar("_shownLength", SkIntToScalar(shown));
//    e.setS32("hideBar", hideBar);
    fAnim.doUserEvent(e);
}
