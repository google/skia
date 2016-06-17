/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"

#include "SkAnnotation.h"
#include "SkData.h"

namespace skiagm {

/** Draws two rectangles. In output formats that support internal links (PDF),
 *  clicking the one labeled "Link to A" should take you to the one labeled
 *  "Target A". Note that you'll need to zoom your PDF viewer in a fair bit in
 *  order for the scrolling to not be blocked by the edge of the document.
 */
class InternalLinksGM : public GM {
public:
    InternalLinksGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:
    virtual SkString onShortName() {
        return SkString("internal_links");
    }

    virtual SkISize onISize() {
        return SkISize::Make(700, 500);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkAutoTUnref<SkData> name(SkData::NewWithCString("target-a"));

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

private:
    /** Draw an arbitrary rectangle at a given location and label it with some
     *  text. */
    void drawLabeledRect(SkCanvas* canvas, const char* text, SkScalar x, SkScalar y) {
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        SkRect rect = SkRect::MakeXYWH(x, y,
                                       SkIntToScalar(50), SkIntToScalar(20));
        canvas->drawRect(rect, paint);

        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(SkIntToScalar(25));
        paint.setColor(SK_ColorBLACK);
        canvas->drawText(text, strlen(text), x, y, paint);
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new InternalLinksGM; }
static GMRegistry reg(MyFactory);

}
