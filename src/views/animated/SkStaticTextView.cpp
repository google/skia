
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidgetViews.h"
#include "SkTextBox.h"

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

SkStaticTextView::SkStaticTextView()
{
    fMargin.set(0, 0);
    fMode = kFixedSize_Mode;
    fSpacingAlign = SkTextBox::kStart_SpacingAlign;

//    init_skin_paint(kStaticText_SkinEnum, &fPaint);
}

SkStaticTextView::~SkStaticTextView()
{
}

void SkStaticTextView::computeSize()
{
    if (fMode == kAutoWidth_Mode)
    {
        SkScalar width = fPaint.measureText(fText.c_str(), fText.size());
        this->setWidth(width + fMargin.fX * 2);
    }
    else if (fMode == kAutoHeight_Mode)
    {
        SkScalar width = this->width() - fMargin.fX * 2;
        int lines = width > 0 ? SkTextLineBreaker::CountLines(fText.c_str(), fText.size(), fPaint, width) : 0;

        this->setHeight(lines * fPaint.getFontSpacing() + fMargin.fY * 2);
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
    if (text == NULL)
        text = "";
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
if (false) { // avoid bit rot, suppress warning
    this->INHERITED::onInflate(dom, node);

    int    index;
    if ((index = dom.findList(node, "mode", "fixed,auto-width,auto-height")) >= 0) {
        this->setMode((Mode)index);
    } else {
        assert_no_attr(dom, node, "mode");
    }

    if ((index = dom.findList(node, "spacing-align", "start,center,end")) >= 0) {
        this->setSpacingAlign((SkTextBox::SpacingAlign)index);
    } else {
        assert_no_attr(dom, node, "spacing-align");
    }

    SkScalar s[2];
    if (dom.findScalars(node, "margin", s, 2)) {
        this->setMargin(s[0], s[1]);
    } else {
        assert_no_attr(dom, node, "margin");
    }

    const char* text = dom.findAttr(node, "text");
    if (text) {
        this->setText(text);
    }

    if ((node = dom.getFirstChild(node, "paint")) != NULL &&
        (node = dom.getFirstChild(node, "screenplay")) != NULL)
    {
// FIXME: Including inflate_paint causes Windows build to fail -- it complains
//  that SKListView::SkListView is undefined.
#if 0
        inflate_paint(dom, node, &fPaint);
#endif
    }
}
}
