/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkAnnotation.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

namespace {

/** Draws two rectangles. In output formats that support internal links (PDF),
 *  clicking the one labeled "Link to A" should take you to the one labeled
 *  "Target A". Note that you'll need to zoom your PDF viewer in a fair bit in
 *  order for the scrolling to not be blocked by the edge of the document.
 */
class InternalLinksGM : public skiagm::GM {
    void onOnceBeforeDraw() override { this->setBGColor(0xFFDDDDDD); }

    SkString getName() const override { return SkString("internal_links"); }

    SkISize getISize() override { return {700, 500}; }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkData> name(SkData::MakeWithCString("target-a"));

        canvas->save();
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        drawLabeledRect(canvas, "Link to A", 0, 0);
        SkRect rect = SkRect::MakeXYWH(0, 0, SkIntToScalar(50), SkIntToScalar(20));
        SkAnnotateLinkToDestination(canvas, rect, name.get());
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(200), SkIntToScalar(200));
        SkPoint point = SkPoint::Make(SkIntToScalar(100), SkIntToScalar(50));
        drawLabeledRect(canvas, "Target A", point.x(), point.y());
        SkAnnotateNamedDestination(canvas, point, name.get());
        canvas->restore();
    }

    /** Draw an arbitrary rectangle at a given location and label it with some
     *  text. */
    void drawLabeledRect(SkCanvas* canvas, const char* text, SkScalar x, SkScalar y) {
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        SkRect rect = SkRect::MakeXYWH(x, y,
                                       SkIntToScalar(50), SkIntToScalar(20));
        canvas->drawRect(rect, paint);

        SkFont font(ToolUtils::DefaultPortableTypeface(), 25);
        paint.setColor(SK_ColorBLACK);
        canvas->drawString(text, x, y, font, paint);
    }
};
}  // namespace

DEF_GM( return new InternalLinksGM; )
