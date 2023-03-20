/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "tools/viewer/ClickHandlerSlide.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

// Renders a string art shape.
// The particular shape rendered can be controlled by clicking horizontally, thereby
// generating an angle from 0 to 1.

class StringArtSlide : public ClickHandlerSlide {
public:
    StringArtSlide() : fAngle(0.305f) { fName = "StringArt"; }

    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkScalar angle = fAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);

        SkPoint center = SkPoint::Make(fSize.width()/2, fSize.height()/2);
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        while (length < (std::min(fSize.width(), fSize.height())/2 - 10.f))
        {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }
        path.close();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(0xFF007700);

        canvas->drawPath(path, paint);
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        fAngle = x/fSize.width();
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click *) override { return false; }

private:
    SkScalar fAngle;
    SkSize fSize;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new StringArtSlide(); )
