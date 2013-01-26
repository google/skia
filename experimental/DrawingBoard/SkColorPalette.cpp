
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGradientShader.h"
#include "SkColorPalette.h"

SkColorPalette::SkColorPalette() {
    fSlotRect = SkRect::MakeWH(SkIntToScalar(50), SkIntToScalar(20));
    fGradientRect = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));
    fSelected = 0;
    fCurrColor = 0xFF000000;

    fColors[0] = SK_ColorWHITE;
    fColors[1] = SK_ColorBLACK;
    fColors[2] = SK_ColorRED;
    fColors[3] = SK_ColorGREEN;
    fColors[4] = SK_ColorBLUE;
}

bool SkColorPalette::onEvent(const SkEvent& evt) {
    return this->INHERITED::onEvent(evt);
}

void SkColorPalette::onDraw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->translate(PalettePadding, PalettePadding);

    for (int i = 0; i < PaletteSlots; ++i) {
        if (fSelected == i) {
            paint.setStrokeWidth(SkIntToScalar(3));
        }
        else {
            paint.setStrokeWidth(1);
        }

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(fSlotRect, paint);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(fColors[i]);
        canvas->drawRect(fSlotRect, paint);
        canvas->translate(0, fSlotRect.height() + PalettePadding);
    }
    paint.setStrokeWidth(0);
    canvas->translate(0, PalettePadding);
    SkPoint p = SkPoint::Make(0,0);
    SkPoint q = SkPoint::Make(this->width(), 0);
    SkPoint pts[] = {p, q};

    SkColor colors[] = { SK_ColorRED, SK_ColorYELLOW, SK_ColorGREEN,
        SK_ColorCYAN, SK_ColorBLUE, SK_ColorMAGENTA,SK_ColorRED};
    SkScalar colorPositions[] = { 0, 0.2, 0.4, 0.5, 0.6, 0.8, 1.0};


    SkShader* shader1 = SkGradientShader::CreateLinear(pts, colors, colorPositions,7,
                                                       SkShader::kMirror_TileMode);
    paint.setShader(shader1)->unref();

    canvas->drawRect(fGradientRect, paint);

    //this->INHERITED::onDraw(canvas);
}

SkView::Click* SkColorPalette::onFindClickHandler(SkScalar x, SkScalar y) {
    return new Click(this);
}

bool SkColorPalette::onClick(SkView::Click* click) {
    SkPoint curr = click->fCurr;
    //SkDebugf("click %f %f \n", curr.fX, curr.fY);
    int selected = selectSlot(curr);
    if (selected >= 0) {
        switch (click->fState) {
            case SkView::Click::kDown_State:
            case SkView::Click::kMoved_State:
            case SkView::Click::kUp_State:
                fSelected = selected;
                fCurrColor = fColors[fSelected];
                break;
            default:
                break;
        }
        return true;
    }
    else{
        //account for padding
        curr.fX -= PalettePadding;
        curr.fY -= 2 * PalettePadding + (fSlotRect.height() + PalettePadding) * PaletteSlots;
        if (curr.fX < 0 || curr.fX > fGradientRect.width() ||
            curr.fY < 0 || curr.fY > fGradientRect.height()) {
            return false;
        }
        else {
            switch (click->fState) {
                case SkView::Click::kDown_State:
                case SkView::Click::kMoved_State:
                case SkView::Click::kUp_State:
                    fColors[fSelected] = selectColorFromGradient(curr);
                    fCurrColor = fColors[fSelected];
                    break;
                default:
                    break;
            }
            return true;
        }
    }
}

void SkColorPalette::onSizeChange() {
    fGradientRect = SkRect::MakeWH(this->width() - 2*PalettePadding,
                                   this->width() - 2*PalettePadding);
    this->INHERITED::onSizeChange();
}

int SkColorPalette::selectSlot(SkPoint& cursorPosition) {
    //account for padding
    cursorPosition.fX -= PalettePadding;
    cursorPosition.fY -= PalettePadding;

    if (cursorPosition.fX < 0 || cursorPosition.fX > fSlotRect.width() ||
        cursorPosition.fY < 0 || cursorPosition.fY > (fSlotRect.height() + PalettePadding) * PaletteSlots) {
        return -1;
    }
    int index = cursorPosition.fY/(fSlotRect.height() + PalettePadding);
    int offset = (int)cursorPosition.fY%((int)fSlotRect.height() + PalettePadding);
    if (offset <= fSlotRect.height()) {
        return index;
    }
    else {
        return -1;
    }
}

SkColor SkColorPalette::selectColorFromGradient(SkPoint& cursorPosition) {
    float h = cursorPosition.fX/fGradientRect.width();
    float s = 1.0 - cursorPosition.fY/fGradientRect.height();
    float v = 1.0;
    float _h,r,g,b;
    float _1, _2, _3;
    int _i;

    _h = h * 6;
    _i = (int)_h;
    _1 = v * (1 - s);
    _2 = v * (1 - s * (_h - _i));
    _3 = v * (1 - s * (1 - (_h - _i)));

    if (_i == 0) {
        r = v;
        g = _3;
        b = _1;
    }
    else if (_i == 1) {
        r = _2;
        g = v;
        b = _1;
    }
    else if (_i == 2) {
        r = _1;
        g = v;
        b = _3;
    }
    else if (_i == 3) {
        r = _1;
        g = _2;
        b = v;
    }
    else if (_i == 4) {
        r = _3;
        g = _1;
        b = v;
    }
    else {
        r = v;
        g = _1;
        b = _2;
    };

    SkColor retval = 0xFF000000;
    retval += ((int)(r * 255) << 16);
    retval += ((int)(g * 255) << 8);
    retval += (int)(b * 255);
    return retval;
}
