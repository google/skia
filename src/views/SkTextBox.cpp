
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTextBox.h"
#include "../core/SkGlyphCache.h"
#include "SkUtils.h"
#include "SkAutoKern.h"

static inline int is_ws(int c)
{
    return !((c - 1) >> 5);
}

static size_t linebreak(const char text[], const char stop[], const SkPaint& paint, SkScalar margin)
{
    const char* start = text;

    SkAutoGlyphCache    ac(paint, NULL);
    SkGlyphCache*       cache = ac.getCache();
    SkFixed             w = 0;
    SkFixed             limit = SkScalarToFixed(margin);
    SkAutoKern          autokern;

    const char* word_start = text;
    int         prevWS = true;

    while (text < stop)
    {
        const char* prevText = text;
        SkUnichar   uni = SkUTF8_NextUnichar(&text);
        int         currWS = is_ws(uni);
        const SkGlyph&  glyph = cache->getUnicharMetrics(uni);

        if (!currWS && prevWS)
            word_start = prevText;
        prevWS = currWS;

        w += autokern.adjust(glyph) + glyph.fAdvanceX;
        if (w > limit)
        {
            if (currWS) // eat the rest of the whitespace
            {
                while (text < stop && is_ws(SkUTF8_ToUnichar(text)))
                    text += SkUTF8_CountUTF8Bytes(text);
            }
            else    // backup until a whitespace (or 1 char)
            {
                if (word_start == start)
                {
                    if (prevText > start)
                        text = prevText;
                }
                else
                    text = word_start;
            }
            break;
        }
    }
    return text - start;
}

int SkTextLineBreaker::CountLines(const char text[], size_t len, const SkPaint& paint, SkScalar width)
{
    const char* stop = text + len;
    int         count = 0;

    if (width > 0)
    {
        do {
            count += 1;
            text += linebreak(text, stop, paint, width);
        } while (text < stop);
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////////

SkTextBox::SkTextBox()
{
    fBox.setEmpty();
    fSpacingMul = SK_Scalar1;
    fSpacingAdd = 0;
    fMode = kLineBreak_Mode;
    fSpacingAlign = kStart_SpacingAlign;
}

void SkTextBox::setMode(Mode mode)
{
    SkASSERT((unsigned)mode < kModeCount);
    fMode = SkToU8(mode);
}

void SkTextBox::setSpacingAlign(SpacingAlign align)
{
    SkASSERT((unsigned)align < kSpacingAlignCount);
    fSpacingAlign = SkToU8(align);
}

void SkTextBox::getBox(SkRect* box) const
{
    if (box)
        *box = fBox;
}

void SkTextBox::setBox(const SkRect& box)
{
    fBox = box;
}

void SkTextBox::setBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    fBox.set(left, top, right, bottom);
}

void SkTextBox::getSpacing(SkScalar* mul, SkScalar* add) const
{
    if (mul)
        *mul = fSpacingMul;
    if (add)
        *add = fSpacingAdd;
}

void SkTextBox::setSpacing(SkScalar mul, SkScalar add)
{
    fSpacingMul = mul;
    fSpacingAdd = add;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SkTextBox::draw(SkCanvas* canvas, const char text[], size_t len, const SkPaint& paint)
{
    SkASSERT(canvas && &paint && (text || len == 0));

    SkScalar marginWidth = fBox.width();

    if (marginWidth <= 0 || len == 0)
        return;

    const char* textStop = text + len;

    SkScalar                x, y, scaledSpacing, height, fontHeight;
    SkPaint::FontMetrics    metrics;

    switch (paint.getTextAlign()) {
    case SkPaint::kLeft_Align:
        x = 0;
        break;
    case SkPaint::kCenter_Align:
        x = SkScalarHalf(marginWidth);
        break;
    default:
        x = marginWidth;
        break;
    }
    x += fBox.fLeft;

    fontHeight = paint.getFontMetrics(&metrics);
    scaledSpacing = SkScalarMul(fontHeight, fSpacingMul) + fSpacingAdd;
    height = fBox.height();

    //  compute Y position for first line
    {
        SkScalar textHeight = fontHeight;

        if (fMode == kLineBreak_Mode && fSpacingAlign != kStart_SpacingAlign)
        {
            int count = SkTextLineBreaker::CountLines(text, textStop - text, paint, marginWidth);
            SkASSERT(count > 0);
            textHeight += scaledSpacing * (count - 1);
        }

        switch (fSpacingAlign) {
        case kStart_SpacingAlign:
            y = 0;
            break;
        case kCenter_SpacingAlign:
            y = SkScalarHalf(height - textHeight);
            break;
        default:
            SkASSERT(fSpacingAlign == kEnd_SpacingAlign);
            y = height - textHeight;
            break;
        }
        y += fBox.fTop - metrics.fAscent;
    }

    for (;;)
    {
        len = linebreak(text, textStop, paint, marginWidth);
        if (y + metrics.fDescent + metrics.fLeading > 0)
            canvas->drawText(text, len, x, y, paint);
        text += len;
        if (text >= textStop)
            break;
        y += scaledSpacing;
        if (y + metrics.fAscent >= height)
            break;
    } 
}

///////////////////////////////////////////////////////////////////////////////

void SkTextBox::setText(const char text[], size_t len, const SkPaint& paint) {
    fText = text;
    fLen = len;
    fPaint = &paint;
}

void SkTextBox::draw(SkCanvas* canvas) {
    this->draw(canvas, fText, fLen, *fPaint);
}

int SkTextBox::countLines() const {
    return SkTextLineBreaker::CountLines(fText, fLen, *fPaint, fBox.width());
}

SkScalar SkTextBox::getTextHeight() const {
    SkScalar spacing = SkScalarMul(fPaint->getTextSize(), fSpacingMul) + fSpacingAdd;
    return this->countLines() * spacing;
}

