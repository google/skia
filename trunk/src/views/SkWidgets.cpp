
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidget.h"
#include "SkCanvas.h"
#include "SkKey.h"
#include "SkParsePaint.h"
#include "SkSystemEventTypes.h"
#include "SkTextBox.h"

#if 0

#ifdef SK_DEBUG
    static void assert_no_attr(const SkDOM& dom, const SkDOM::Node* node, const char attr[])
    {
        const char* value = dom.findAttr(node, attr);
        if (value)
            SkDebugf("unknown attribute %s=\"%s\"\n", attr, value);
    }
#else
    #define assert_no_attr(dom, node, attr)
#endif

#include "SkAnimator.h"
#include "SkTime.h"

///////////////////////////////////////////////////////////////////////////////

enum SkinType {
    kPushButton_SkinType,
    kStaticText_SkinType,

    kSkinTypeCount
};

struct SkinSuite {
    SkinSuite();
    ~SkinSuite()
    {
        for (int i = 0; i < kSkinTypeCount; i++)
            delete fAnimators[i];
    }

    SkAnimator*    get(SkinType);

private:
    SkAnimator*    fAnimators[kSkinTypeCount];
};

SkinSuite::SkinSuite()
{
    static const char kSkinPath[] = "skins/";

    static const char* gSkinNames[] = {
        "pushbutton_skin.xml",
        "statictext_skin.xml"
    };

    for (unsigned i = 0; i < SK_ARRAY_COUNT(gSkinNames); i++)
    {
        size_t        len = strlen(gSkinNames[i]);
        SkString    path(sizeof(kSkinPath) - 1 + len);

        memcpy(path.writable_str(), kSkinPath, sizeof(kSkinPath) - 1);
        memcpy(path.writable_str() + sizeof(kSkinPath) - 1, gSkinNames[i], len);

        fAnimators[i] = new SkAnimator;
        if (!fAnimators[i]->decodeURI(path.c_str()))
        {
            delete fAnimators[i];
            fAnimators[i] = NULL;
        }
    }
}

SkAnimator* SkinSuite::get(SkinType st)
{
    SkASSERT((unsigned)st < kSkinTypeCount);
    return fAnimators[st];
}

static SkinSuite* gSkinSuite;

static SkAnimator* get_skin_animator(SkinType st)
{
#if 0
    if (gSkinSuite == NULL)
        gSkinSuite = new SkinSuite;
    return gSkinSuite->get(st);
#else
    return NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////////

void SkWidget::Init()
{
}

void SkWidget::Term()
{
    delete gSkinSuite;
}

void SkWidget::onEnabledChange()
{
    this->inval(NULL);
}

void SkWidget::postWidgetEvent()
{
    if (!fEvent.isType("") && this->hasListeners())
    {
        this->prepareWidgetEvent(&fEvent);
        this->postToListeners(fEvent);
    }
}

void SkWidget::prepareWidgetEvent(SkEvent*)
{
    // override in subclass to add any additional fields before posting
}

void SkWidget::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    if ((node = dom.getFirstChild(node, "event")) != NULL)
        fEvent.inflate(dom, node);
}

///////////////////////////////////////////////////////////////////////////////

size_t SkHasLabelWidget::getLabel(SkString* str) const
{
    if (str)
        *str = fLabel;
    return fLabel.size();
}

size_t SkHasLabelWidget::getLabel(char buffer[]) const
{
    if (buffer)
        memcpy(buffer, fLabel.c_str(), fLabel.size());
    return fLabel.size();
}

void SkHasLabelWidget::setLabel(const SkString& str)
{
    this->setLabel(str.c_str(), str.size());
}

void SkHasLabelWidget::setLabel(const char label[])
{
    this->setLabel(label, strlen(label));
}

void SkHasLabelWidget::setLabel(const char label[], size_t len)
{
    if (!fLabel.equals(label, len))
    {
        fLabel.set(label, len);
        this->onLabelChange();
    }
}

void SkHasLabelWidget::onLabelChange()
{
    // override in subclass
}

void SkHasLabelWidget::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    const char* text = dom.findAttr(node, "label");
    if (text)
        this->setLabel(text);
}

/////////////////////////////////////////////////////////////////////////////////////

void SkButtonWidget::setButtonState(State state)
{
    if (fState != state)
    {
        fState = state;
        this->onButtonStateChange();
    }
}

void SkButtonWidget::onButtonStateChange()
{
    this->inval(NULL);
}

void SkButtonWidget::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    int    index;
    if ((index = dom.findList(node, "buttonState", "off,on,unknown")) >= 0)
        this->setButtonState((State)index);
}

/////////////////////////////////////////////////////////////////////////////////////

bool SkPushButtonWidget::onEvent(const SkEvent& evt)
{
    if (evt.isType(SK_EventType_Key) && evt.getFast32() == kOK_SkKey)
    {
        this->postWidgetEvent();
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

static const char* computeAnimatorState(int enabled, int focused, SkButtonWidget::State state)
{
    if (!enabled)
        return "disabled";
    if (state == SkButtonWidget::kOn_State)
    {
        SkASSERT(focused);
        return "enabled-pressed";
    }
    if (focused)
        return "enabled-focused";
    return "enabled";
}

#include "SkBlurMaskFilter.h"
#include "SkEmbossMaskFilter.h"

static void create_emboss(SkPaint* paint, SkScalar radius, bool focus, bool pressed)
{
    SkEmbossMaskFilter::Light    light;

    light.fDirection[0] = SK_Scalar1/2;
    light.fDirection[1] = SK_Scalar1/2;
    light.fDirection[2] = SK_Scalar1/3;
    light.fAmbient        = 0x48;
    light.fSpecular        = 0x80;

    if (pressed)
    {
        light.fDirection[0] = -light.fDirection[0];
        light.fDirection[1] = -light.fDirection[1];
    }
    if (focus)
        light.fDirection[2] += SK_Scalar1/4;

    paint->setMaskFilter(new SkEmbossMaskFilter(light, radius))->unref();
}

void SkPushButtonWidget::onDraw(SkCanvas* canvas)
{
    this->INHERITED::onDraw(canvas);

    SkString label;
    this->getLabel(&label);

    SkAnimator* anim = get_skin_animator(kPushButton_SkinType);

    if (anim)
    {
        SkEvent    evt("user");

        evt.setString("id", "prime");
        evt.setScalar("prime-width", this->width());
        evt.setScalar("prime-height", this->height());
        evt.setString("prime-text", label);
        evt.setString("prime-state", computeAnimatorState(this->isEnabled(), this->hasFocus(), this->getButtonState()));

        (void)anim->doUserEvent(evt);
        SkPaint paint;
        anim->draw(canvas, &paint, SkTime::GetMSecs());
    }
    else
    {
        SkRect    r;
        SkPaint    p;

        r.set(0, 0, this->width(), this->height());
        p.setAntiAliasOn(true);
        p.setColor(SK_ColorBLUE);
        create_emboss(&p, SkIntToScalar(12)/5, this->hasFocus(), this->getButtonState() == kOn_State);
        canvas->drawRoundRect(r, SkScalarHalf(this->height()), SkScalarHalf(this->height()), p);
        p.setMaskFilter(NULL);

        p.setTextAlign(SkPaint::kCenter_Align);

        SkTextBox    box;
        box.setMode(SkTextBox::kOneLine_Mode);
        box.setSpacingAlign(SkTextBox::kCenter_SpacingAlign);
        box.setBox(0, 0, this->width(), this->height());

//        if (this->getButtonState() == kOn_State)
//            p.setColor(SK_ColorRED);
//        else
            p.setColor(SK_ColorWHITE);

        box.draw(canvas, label.c_str(), label.size(), p);
    }
}

SkView::Click* SkPushButtonWidget::onFindClickHandler(SkScalar x, SkScalar y)
{
    this->acceptFocus();
    return new Click(this);
}

bool SkPushButtonWidget::onClick(Click* click)
{
    SkRect    r;
    State    state = kOff_State;

    this->getLocalBounds(&r);
    if (r.contains(click->fCurr))
    {
        if (click->fState == Click::kUp_State)
            this->postWidgetEvent();
        else
            state = kOn_State;
    }
    this->setButtonState(state);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

SkStaticTextView::SkStaticTextView(U32 flags) : SkView(flags)
{
    fMargin.set(0, 0);
    fMode = kFixedSize_Mode;
    fSpacingAlign = SkTextBox::kStart_SpacingAlign;
}

SkStaticTextView::~SkStaticTextView()
{
}

void SkStaticTextView::computeSize()
{
    if (fMode == kAutoWidth_Mode)
    {
        SkScalar width = fPaint.measureText(fText.c_str(), fText.size(), NULL, NULL);
        this->setWidth(width + fMargin.fX * 2);
    }
    else if (fMode == kAutoHeight_Mode)
    {
        SkScalar width = this->width() - fMargin.fX * 2;
        int lines = width > 0 ? SkTextLineBreaker::CountLines(fText.c_str(), fText.size(), fPaint, width) : 0;

        SkScalar    before, after;
        (void)fPaint.measureText(0, NULL, &before, &after);

        this->setHeight(lines * (after - before) + fMargin.fY * 2);
    }
}

void SkStaticTextView::setMode(Mode mode)
{
    SkASSERT((unsigned)mode < kModeCount);

    if (fMode != mode)
    {
        fMode = SkToU8(mode);
        this->computeSize();
    }
}

void SkStaticTextView::setSpacingAlign(SkTextBox::SpacingAlign align)
{
    fSpacingAlign = SkToU8(align);
    this->inval(NULL);
}

void SkStaticTextView::getMargin(SkPoint* margin) const
{
    if (margin)
        *margin = fMargin;
}

void SkStaticTextView::setMargin(SkScalar dx, SkScalar dy)
{
    if (fMargin.fX != dx || fMargin.fY != dy)
    {
        fMargin.set(dx, dy);
        this->computeSize();
        this->inval(NULL);
    }
}

size_t SkStaticTextView::getText(SkString* text) const
{
    if (text)
        *text = fText;
    return fText.size();
}

size_t SkStaticTextView::getText(char text[]) const
{
    if (text)
        memcpy(text, fText.c_str(), fText.size());
    return fText.size();
}

void SkStaticTextView::setText(const SkString& text)
{
    this->setText(text.c_str(), text.size());
}

void SkStaticTextView::setText(const char text[])
{
    this->setText(text, strlen(text));
}

void SkStaticTextView::setText(const char text[], size_t len)
{
    if (!fText.equals(text, len))
    {
        fText.set(text, len);
        this->computeSize();
        this->inval(NULL);
    }
}

void SkStaticTextView::getPaint(SkPaint* paint) const
{
    if (paint)
        *paint = fPaint;
}

void SkStaticTextView::setPaint(const SkPaint& paint)
{
    if (fPaint != paint)
    {
        fPaint = paint;
        this->computeSize();
        this->inval(NULL);
    }
}

void SkStaticTextView::onDraw(SkCanvas* canvas)
{
    this->INHERITED::onDraw(canvas);

    if (fText.isEmpty())
        return;

    SkTextBox    box;

    box.setMode(fMode == kAutoWidth_Mode ? SkTextBox::kOneLine_Mode : SkTextBox::kLineBreak_Mode);
    box.setSpacingAlign(this->getSpacingAlign());
    box.setBox(fMargin.fX, fMargin.fY, this->width() - fMargin.fX, this->height() - fMargin.fY);
    box.draw(canvas, fText.c_str(), fText.size(), fPaint);
}

void SkStaticTextView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    int    index;
    if ((index = dom.findList(node, "mode", "fixed,auto-width,auto-height")) >= 0)
        this->setMode((Mode)index);
    else
        assert_no_attr(dom, node, "mode");

    if ((index = dom.findList(node, "spacing-align", "start,center,end")) >= 0)
        this->setSpacingAlign((SkTextBox::SpacingAlign)index);
    else
        assert_no_attr(dom, node, "mode");

    SkScalar s[2];
    if (dom.findScalars(node, "margin", s, 2))
        this->setMargin(s[0], s[1]);
    else
        assert_no_attr(dom, node, "margin");

    const char* text = dom.findAttr(node, "text");
    if (text)
        this->setText(text);

    if ((node = dom.getFirstChild(node, "paint")) != NULL)
        SkPaint_Inflate(&fPaint, dom, node);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkImageDecoder.h"

SkBitmapView::SkBitmapView(U32 flags) : SkView(flags)
{
}

SkBitmapView::~SkBitmapView()
{
}

bool SkBitmapView::getBitmap(SkBitmap* bitmap) const
{
    if (bitmap)
        *bitmap = fBitmap;
    return fBitmap.getConfig() != SkBitmap::kNo_Config;
}

void SkBitmapView::setBitmap(const SkBitmap* bitmap, bool viewOwnsPixels)
{
    if (bitmap)
    {
        fBitmap = *bitmap;
        fBitmap.setOwnsPixels(viewOwnsPixels);
    }
}

bool SkBitmapView::loadBitmapFromFile(const char path[])
{
    SkBitmap    bitmap;

    if (SkImageDecoder::DecodeFile(path, &bitmap))
    {
        this->setBitmap(&bitmap, true);
        bitmap.setOwnsPixels(false);
        return true;
    }
    return false;
}

void SkBitmapView::onDraw(SkCanvas* canvas)
{
    if (fBitmap.getConfig() != SkBitmap::kNo_Config &&
        fBitmap.width() && fBitmap.height())
    {
        SkAutoCanvasRestore    restore(canvas, true);
        SkPaint                p;

        p.setFilterType(SkPaint::kBilinear_FilterType);
        canvas->scale(    this->width() / fBitmap.width(),
                        this->height() / fBitmap.height(),
                        0, 0);
        canvas->drawBitmap(fBitmap, 0, 0, p);
    }
}

void SkBitmapView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    const char* src = dom.findAttr(node, "src");
    if (src)
        (void)this->loadBitmapFromFile(src);
}

#endif

