
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidgetViews.h"
#include "SkAnimator.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkSystemEventTypes.h"

/*
I have moved this to SkWidgetViews.h
enum SkinEnum {
    kButton_SkinEnum,
    kProgress_SkinEnum,
    kScroll_SkinEnum,
    kStaticText_SkinEnum,

    kSkinEnumCount
};
*/

const char* get_skin_enum_path(SkinEnum se)
{
    SkASSERT((unsigned)se < kSkinEnumCount);

    static const char* gSkinPaths[] = {
            "common/default/default/skins/border3.xml",
            "common/default/default/skins/button.xml",
            "common/default/default/skins/progressBar.xml",
            "common/default/default/skins/scrollBar.xml",
            "common/default/default/skins/statictextpaint.xml"
    };

    return gSkinPaths[se];
}

void init_skin_anim(const char path[], SkAnimator* anim) {
    SkASSERT(path && anim);

    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    if (!stream.get()) {
        SkDEBUGF(("init_skin_anim: loading skin failed <%s>\n", path));
        sk_throw();
    }

    if (!anim->decodeStream(stream)) {
        SkDEBUGF(("init_skin_anim: decoding skin failed <%s>\n", path));
        sk_throw();
    }
}

void init_skin_anim(SkinEnum se, SkAnimator* anim)
{
    init_skin_anim(get_skin_enum_path(se), anim);
}

void init_skin_paint(SkinEnum se, SkPaint* paint)
{
    SkASSERT(paint);

    SkAnimator    anim;
    SkCanvas    canvas;

    init_skin_anim(se, &anim);
    anim.draw(&canvas, paint, 0);
}

void inflate_paint(const SkDOM& dom, const SkDOM::Node* node, SkPaint* paint)
{
    SkASSERT(paint);

    SkAnimator    anim;
    SkCanvas    canvas;

    if (!anim.decodeDOM(dom, node))
    {
        SkDEBUGF(("inflate_paint: decoding dom failed\n"));
        SkDEBUGCODE(dom.dump(node);)
        sk_throw();
    }
    anim.draw(&canvas, paint, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

SkWidgetView::SkWidgetView() : SkView(SkView::kFocusable_Mask | SkView::kEnabled_Mask)
{
}

const char* SkWidgetView::getLabel() const
{
    return fLabel.c_str();
}

void SkWidgetView::getLabel(SkString* label) const
{
    if (label)
        *label = fLabel;
}

void SkWidgetView::setLabel(const char label[])
{
    this->setLabel(label, label ? strlen(label) : 0);
}

void SkWidgetView::setLabel(const char label[], size_t len)
{
    if ((label == nullptr && fLabel.size() != 0) || !fLabel.equals(label, len))
    {
        SkString    tmp(label, len);

        this->onLabelChange(fLabel.c_str(), tmp.c_str());
        fLabel.swap(tmp);
    }
}

void SkWidgetView::setLabel(const SkString& label)
{
    if (fLabel != label)
    {
        this->onLabelChange(fLabel.c_str(), label.c_str());
        fLabel = label;
    }
}

bool SkWidgetView::postWidgetEvent()
{
    if (!fEvent.isType(""))
    {
        SkEvent    evt(fEvent);    // make a copy since onPrepareWidgetEvent may edit the event

        if (this->onPrepareWidgetEvent(&evt))
        {
            SkDEBUGCODE(evt.dump("SkWidgetView::postWidgetEvent");)

            this->postToListeners(evt);    // wonder if this should return true if there are > 0 listeners...
            return true;
        }
    }
    return false;
}

/*virtual*/ void SkWidgetView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    const char* label = dom.findAttr(node, "label");
    if (label)
        this->setLabel(label);

    if ((node = dom.getFirstChild(node, "event")) != nullptr)
        fEvent.inflate(dom, node);
}

/*virtual*/ void SkWidgetView::onLabelChange(const char oldLabel[], const char newLabel[])
{
    this->inval(nullptr);
}

static const char gWidgetEventSinkIDSlotName[] = "sk-widget-sinkid-slot";

/*virtual*/ bool SkWidgetView::onPrepareWidgetEvent(SkEvent* evt)
{
    evt->setS32(gWidgetEventSinkIDSlotName, this->getSinkID());
    return true;
}

SkEventSinkID SkWidgetView::GetWidgetEventSinkID(const SkEvent& evt)
{
    int32_t    sinkID;

    return evt.findS32(gWidgetEventSinkIDSlotName, &sinkID) ? (SkEventSinkID)sinkID : 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*virtual*/ bool SkButtonView::onEvent(const SkEvent& evt)
{
    if (evt.isType(SK_EventType_Key) && evt.getFast32() == kOK_SkKey)
    {
        this->postWidgetEvent();
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkCheckButtonView::SkCheckButtonView() : fCheckState(kOff_CheckState)
{
}

void SkCheckButtonView::setCheckState(CheckState state)
{
    SkASSERT((unsigned)state <= kUnknown_CheckState);

    if (fCheckState != state)
    {
        this->onCheckStateChange(this->getCheckState(), state);
        fCheckState = SkToU8(state);
    }
}

/*virtual*/ void SkCheckButtonView::onCheckStateChange(CheckState oldState, CheckState newState)
{
    this->inval(nullptr);
}

/*virtual*/ void SkCheckButtonView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    int index = dom.findList(node, "check-state", "off,on,unknown");
    if (index >= 0)
        this->setCheckState((CheckState)index);
}

static const char gCheckStateSlotName[] = "sk-checkbutton-check-slot";

/*virtual*/ bool SkCheckButtonView::onPrepareWidgetEvent(SkEvent* evt)
{
    // could check if we're "disabled", and return false...

    evt->setS32(gCheckStateSlotName, this->getCheckState());
    return true;
}

bool SkCheckButtonView::GetWidgetEventCheckState(const SkEvent& evt, CheckState* state)
{
    int32_t    state32;

    if (evt.findS32(gCheckStateSlotName, &state32))
    {
        if (state)
            *state = (CheckState)state32;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkTime.h"
#include <stdio.h>

class SkAnimButtonView : public SkButtonView {
public:
    SkAnimButtonView()
    {
        fAnim.setHostEventSink(this);
        init_skin_anim(kButton_SkinEnum, &fAnim);
    }

protected:
    virtual void onLabelChange(const char oldLabel[], const char newLabel[])
    {
        this->INHERITED::onLabelChange(oldLabel, newLabel);

        SkEvent evt("user");
        evt.setString("id", "setLabel");
        evt.setString("LABEL", newLabel);
        fAnim.doUserEvent(evt);
    }

    virtual void onFocusChange(bool gainFocus)
    {
        this->INHERITED::onFocusChange(gainFocus);

        SkEvent evt("user");
        evt.setString("id", "setFocus");
        evt.setS32("FOCUS", gainFocus);
        fAnim.doUserEvent(evt);
    }

    virtual void onSizeChange()
    {
        this->INHERITED::onSizeChange();

        SkEvent evt("user");
        evt.setString("id", "setDim");
        evt.setScalar("dimX", this->width());
        evt.setScalar("dimY", this->height());
        fAnim.doUserEvent(evt);
    }

    virtual void onDraw(SkCanvas* canvas)
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

    virtual bool onEvent(const SkEvent& evt)
    {
        if (evt.isType(SK_EventType_Inval))
        {
            this->inval(nullptr);
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

    virtual bool onPrepareWidgetEvent(SkEvent* evt)
    {
        if (this->INHERITED::onPrepareWidgetEvent(evt))
        {
            SkEvent    e("user");
            e.setString("id", "handlePress");
            (void)fAnim.doUserEvent(e);
            return true;
        }
        return false;
    }

private:
    SkAnimator    fAnim;

    typedef SkButtonView INHERITED;
};

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

SkView* SkWidgetFactory(const char name[])
{
    if (name == nullptr)
        return nullptr;

    // must be in the same order as the SkSkinWidgetEnum is declared
    static const char* gNames[] = {
        "sk-border",
        "sk-button",
        "sk-image",
        "sk-list",
        "sk-progress",
        "sk-scroll",
        "sk-text"

    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); i++)
        if (!strcmp(gNames[i], name))
            return SkWidgetFactory((SkWidgetEnum)i);

    return nullptr;
}

#include "SkImageView.h"
#include "SkProgressBarView.h"
#include "SkScrollBarView.h"
#include "SkBorderView.h"

SkView* SkWidgetFactory(SkWidgetEnum sw)
{
    switch (sw) {
    case kBorder_WidgetEnum:
        return new SkBorderView;
    case kButton_WidgetEnum:
        return new SkAnimButtonView;
    case kImage_WidgetEnum:
        return new SkImageView;
    case kList_WidgetEnum:
        return new SkListView;
    case kProgress_WidgetEnum:
        return new SkProgressBarView;
    case kScroll_WidgetEnum:
        return new SkScrollBarView;
    case kText_WidgetEnum:
        return new SkStaticTextView;
    default:
        SkDEBUGFAIL("unknown enum passed to SkWidgetFactory");
        break;
    }
    return nullptr;
}
