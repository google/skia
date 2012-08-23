
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkWidget.h"
#include "SkCanvas.h"
#include "SkInterpolator.h"
#include "SkTime.h"
#include "SkParsePaint.h"

#if 0
SkWidgetView::SkWidgetView(U32 flags) : SkView(flags)
{
}

SkWidgetView::~SkWidgetView()
{
}

const char* SkWidgetView::GetEventType()
{
    return "SkWidgetView";
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class SkTextView::Interp {
public:
    Interp(const SkString& old, SkMSec now, SkMSec dur, AnimaDir dir) : fOldText(old), fInterp(1, 2)
    {
        SkScalar x = 0;
        fInterp.setKeyFrame(0, now, &x, 0);
        x = SK_Scalar1;
        if (dir == kBackward_AnimDir)
            x = -x;
        fInterp.setKeyFrame(1, now + dur, &x);
    }
    bool draw(SkCanvas* canvas, const SkString& newText, SkScalar x, SkScalar y, SkPaint& paint)
    {
        SkScalar scale;

        if (fInterp.timeToValues(SkTime::GetMSecs(), &scale) == SkInterpolator::kFreezeEnd_Result)
        {
            canvas->drawText(newText.c_str(), newText.size(), x, y, paint);
            return false;
        }
        else
        {
            U8 alpha = paint.getAlpha();
            SkScalar above, below;
            (void)paint.measureText(NULL, 0, &above, &below);
            SkScalar height = below - above;
            SkScalar dy = SkScalarMul(height, scale);
            if (scale < 0)
                height = -height;

            // draw the old
            paint.setAlpha((U8)SkScalarMul(alpha, SK_Scalar1 - SkScalarAbs(scale)));
            canvas->drawText(fOldText.c_str(), fOldText.size(), x, y - dy, paint);
            // draw the new
            paint.setAlpha((U8)SkScalarMul(alpha, SkScalarAbs(scale)));
            canvas->drawText(newText.c_str(), newText.size(), x, y + height - dy, paint);
            // restore the paint
            paint.setAlpha(alpha);
            return true;
        }
    }

private:
    SkString        fOldText;
    SkInterpolator    fInterp;
};

SkTextView::SkTextView(U32 flags) : SkView(flags), fInterp(NULL), fDoInterp(false)
{
    fMargin.set(0, 0);
}

SkTextView::~SkTextView()
{
    delete fInterp;
}

void SkTextView::getText(SkString* str) const
{
    if (str)
        str->set(fText);
}

void SkTextView::setText(const char text[], AnimaDir dir)
{
    if (!fText.equals(text))
    {
        SkString tmp(text);
        this->privSetText(tmp, dir);
    }
}

void SkTextView::setText(const char text[], size_t len, AnimaDir dir)
{
    if (!fText.equals(text))
    {
        SkString tmp(text, len);
        this->privSetText(tmp, dir);
    }
}

void SkTextView::setText(const SkString& src, AnimaDir dir)
{
    if (fText != src)
        this->privSetText(src, dir);
}

void SkTextView::privSetText(const SkString& src, AnimaDir dir)
{
    SkASSERT(fText != src);

    if (fDoInterp)
    {
        if (fInterp)
            delete fInterp;
        fInterp = new Interp(fText, SkTime::GetMSecs(), 500, dir);
    }
    fText = src;
    this->inval(NULL);
}

/////////////////////////////////////////////////////////////////

void SkTextView::getMargin(SkPoint* margin) const
{
    if (margin)
        *margin = fMargin;
}

void SkTextView::setMargin(const SkPoint& margin)
{
    if (fMargin != margin)
    {
        fMargin = margin;
        this->inval(NULL);
    }
}

void SkTextView::onDraw(SkCanvas* canvas)
{
    this->INHERITED::onDraw(canvas);

    if (fText.size() == 0)
        return;

    SkPaint::Align    align = fPaint.getTextAlign();
    SkScalar        x, y;

    switch (align) {
    case SkPaint::kLeft_Align:
        x = fMargin.fX;
        break;
    case SkPaint::kCenter_Align:
        x = SkScalarHalf(this->width());
        break;
    default:
        SkASSERT(align == SkPaint::kRight_Align);
        x = this->width() - fMargin.fX;
        break;
    }

    fPaint.measureText(NULL, 0, &y, NULL);
    y = fMargin.fY - y;

    if (fInterp)
    {
        if (fInterp->draw(canvas, fText, x, y, fPaint))
            this->inval(NULL);
        else
        {
            delete fInterp;
            fInterp = NULL;
        }
    }
    else
        canvas->drawText(fText.c_str(), fText.size(), x, y, fPaint);
}

//////////////////////////////////////////////////////////////////////////////////////

void SkTextView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    const char* text = dom.findAttr(node, "text");
    if (text)
        this->setText(text);

    SkPoint    margin;
    if (dom.findScalars(node, "margin", (SkScalar*)&margin, 2))
        this->setMargin(margin);
    (void)dom.findBool(node, "do-interp", &fDoInterp);

    SkPaint_Inflate(&fPaint, dom, node);
}

//////////////////////////////////////////////////////////////////////////////////////

SkSliderView::SkSliderView(U32 flags) : SkWidgetView(flags)
{
    fValue = 0;
    fMax = 0;
}

static U16 actual_value(U16CPU value, U16CPU max)
{
    return SkToU16(SkMax32(0, SkMin32(value, max)));
}

void SkSliderView::setMax(U16CPU max)
{
    if (fMax != max)
    {
        fMax = SkToU16(max);
        if (fValue > 0)
            this->inval(NULL);
    }
}

void SkSliderView::setValue(U16CPU value)
{
    if (fValue != value)
    {
        U16 prev = actual_value(fValue, fMax);
        U16 next = actual_value(value, fMax);

        fValue = SkToU16(value);
        if (prev != next)
        {
            this->inval(NULL);

            if (this->hasListeners())
            {
                SkEvent    evt;

                evt.setType(SkWidgetView::GetEventType());
                evt.setFast32(this->getSinkID());
                evt.setS32("sliderValue", next);
                this->postToListeners(evt);
            }
        }
    }
}

#include "SkGradientShader.h"

static void setgrad(SkPaint* paint, const SkRect& r)
{
    SkPoint    pts[2];
    SkColor    colors[2];

#if 0
    pts[0].set(r.fLeft, r.fTop);
    pts[1].set(r.fLeft + r.height(), r.fBottom);
#else
    pts[0].set(r.fRight, r.fBottom);
    pts[1].set(r.fRight - r.height(), r.fTop);
#endif
    colors[0] = SK_ColorBLUE;
    colors[1] = SK_ColorWHITE;

    paint->setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kMirror_TileMode))->unref();
}

void SkSliderView::onDraw(SkCanvas* canvas)
{
    this->INHERITED::onDraw(canvas);

    U16CPU value = SkMax32(0, SkMin32(fValue, fMax));

    SkRect    r;
    SkPaint    p;

    r.set(0, 0, this->width(), this->height());

    p.setAntiAliasOn(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    r.inset(SK_Scalar1/2, SK_Scalar1/2);
    canvas->drawRect(r, p);

    if (fMax)
    {
        SkFixed percent = SkFixedDiv(value, fMax);

        r.inset(SK_Scalar1/2, SK_Scalar1/2);
        r.fRight = r.fLeft + SkScalarMul(r.width(), SkFixedToScalar(percent));
        p.setStyle(SkPaint::kFill_Style);
        setgrad(&p, r);
        canvas->drawRect(r, p);
    }

#if 0
    r.set(0, 0, this->width(), this->height());
    r.inset(SK_Scalar1, SK_Scalar1);
    r.inset(r.width()/2, 0);
    p.setColor(SK_ColorBLACK);
    canvas->drawLine(*(SkPoint*)&r.fLeft, *(SkPoint*)&r.fRight, p);
#endif
}

SkView::Click* SkSliderView::onFindClickHandler(SkScalar x, SkScalar y)
{
    return new Click(this);
}

bool SkSliderView::onClick(Click* click)
{
    if (fMax)
    {
        SkScalar percent = SkScalarDiv(click->fCurr.fX + SK_Scalar1, this->width() - SK_Scalar1*2);
        percent = SkMaxScalar(0, SkMinScalar(percent, SK_Scalar1));
        this->setValue(SkScalarRound(percent * fMax));
        return true;
    }
    return false;
}

#endif

