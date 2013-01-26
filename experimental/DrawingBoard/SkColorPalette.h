
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorPalette_DEFINED
#define SkColorPalette_DEFINED

#define PaletteSlots 5
#define PalettePadding 5
class SkColorPalette : public SkView {
public:
    SkColorPalette();
    SkColor getColor() { return fCurrColor; }
protected:
    virtual bool onEvent(const SkEvent& evt);
    virtual void onDraw(SkCanvas* canvas);
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y);
    virtual bool onClick(SkView::Click* click);
    virtual void onSizeChange();
private:
    int selectSlot(SkPoint& cursorPosition);
    SkColor selectColorFromGradient(SkPoint& cursorPosition);
    int     fSelected;
    SkRect  fGradientRect;
    SkRect  fSlotRect;
    SkColor fCurrColor;
    SkColor fColors[PaletteSlots];
    typedef SkView INHERITED;
};

#endif
