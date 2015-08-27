
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBorderView.h"
#include "SkAnimator.h"
#include "SkWidgetViews.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"
#include "SkStackViewLayout.h"

SkBorderView::SkBorderView() : fLeft(SkIntToScalar(0)),
                               fRight(SkIntToScalar(0)),
                               fTop(SkIntToScalar(0)),
                               fBottom(SkIntToScalar(0))
{
    fAnim.setHostEventSink(this);
    init_skin_anim(kBorder_SkinEnum, &fAnim);
}

SkBorderView::~SkBorderView()
{

}

void SkBorderView::setSkin(const char skin[])
{
    init_skin_anim(skin, &fAnim);
}

/* virtual */ void SkBorderView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);
}

/*virtual*/ void SkBorderView::onSizeChange()
{
    this->INHERITED::onSizeChange();
    SkEvent evt("user");
    evt.setString("id", "setDim");
    evt.setScalar("dimX", this->width());
    evt.setScalar("dimY", this->height());
    fAnim.doUserEvent(evt);
}

/*virtual*/ void SkBorderView::onDraw(SkCanvas* canvas)
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

/*virtual*/ bool SkBorderView::onEvent(const SkEvent& evt)
{
    if (evt.isType(SK_EventType_Inval))
    {
        this->inval(nullptr);
        return true;
    }
    if (evt.isType("recommendDim"))
    {
        evt.findScalar("leftMargin", &fLeft);
        evt.findScalar("rightMargin", &fRight);
        evt.findScalar("topMargin", &fTop);
        evt.findScalar("bottomMargin", &fBottom);

        //setup_views.cpp uses SkView::Layout instead of SkStackViewLayout
        //but that gives me an error
        SkStackViewLayout* layout;
        fMargin.set(fLeft, fTop, fRight, fBottom);
        if (this->getLayout())
        {
            layout = (SkStackViewLayout*)this->getLayout();
            layout->setMargin(fMargin);
        }
        else
        {
            layout = new SkStackViewLayout;
            layout->setMargin(fMargin);
            this->setLayout(layout)->unref();
        }
        this->invokeLayout();
    }
    return this->INHERITED::onEvent(evt);
}
