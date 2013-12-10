/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkTypeface.h"

namespace skiagm {

class ColorEmojiGM : public GM {
public:
    ColorEmojiGM() {
        fTypeface = NULL;
    }

    ~ColorEmojiGM() {
        SkSafeUnref(fTypeface);
    }
protected:
    virtual void onOnceBeforeDraw() SK_OVERRIDE {

        SkString filename(INHERITED::gResourcePath);
        filename.append("/Funkster.ttf");

        SkAutoTUnref<SkFILEStream> stream(new SkFILEStream(filename.c_str()));
        if (!stream->isValid()) {
            SkDebugf("Could not find Funkster.ttf, please set --resourcePath correctly.\n");
            return;
        }

        fTypeface = SkTypeface::CreateFromStream(stream);
    }

    virtual SkString onShortName() {
        return SkString("coloremoji");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        paint.setTypeface(fTypeface);

        const char* text = "hamburgerfons";

        // draw text at different point sizes
        const int textSize[] = { 10, 30, 50 };
        const int textYOffset[] = { 10, 40, 100};
        SkASSERT(sizeof(textSize) == sizeof(textYOffset));
        for (size_t y = 0; y < sizeof(textSize) / sizeof(int); ++y) {
            paint.setTextSize(SkIntToScalar(textSize[y]));
            canvas->drawText(text, strlen(text), 10, SkIntToScalar(textYOffset[y]), paint);
        }

        // setup work needed to draw text with different clips
        canvas->translate(10, 160);
        paint.setTextSize(40);

        // compute the bounds of the text
        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        const SkScalar boundsHalfWidth = bounds.width() * SK_ScalarHalf;
        const SkScalar boundsHalfHeight = bounds.height() * SK_ScalarHalf;
        const SkScalar boundsQuarterWidth = boundsHalfWidth * SK_ScalarHalf;
        const SkScalar boundsQuarterHeight = boundsHalfHeight * SK_ScalarHalf;

        SkRect upperLeftClip = SkRect::MakeXYWH(bounds.left(), bounds.top(),
                                                boundsHalfWidth, boundsHalfHeight);
        SkRect lowerRightClip = SkRect::MakeXYWH(bounds.centerX(), bounds.centerY(),
                                                 boundsHalfWidth, boundsHalfHeight);
        SkRect interiorClip = bounds;
        interiorClip.inset(boundsQuarterWidth, boundsQuarterHeight);

        const SkRect clipRects[] = { bounds, upperLeftClip, lowerRightClip, interiorClip };

        SkPaint clipHairline;
        clipHairline.setColor(SK_ColorWHITE);
        clipHairline.setStyle(SkPaint::kStroke_Style);

        for (size_t x = 0; x < sizeof(clipRects) / sizeof(SkRect); ++x) {
            canvas->save();
            canvas->drawRect(clipRects[x], clipHairline);
            paint.setAlpha(0x20);
            canvas->drawText(text, strlen(text), 0, 0, paint);
            canvas->clipRect(clipRects[x]);
            paint.setAlpha(0xFF);
            canvas->drawText(text, strlen(text), 0, 0, paint);
            canvas->restore();
            canvas->translate(0, bounds.height() + SkIntToScalar(25));
        }
    }

private:
    SkTypeface* fTypeface;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

#if !defined(SK_BUILD_FOR_ANDROID)
// fail for now until the appropriate freetype changes are submitted
static GM* MyFactory(void*) { return new ColorEmojiGM; }
static GMRegistry reg(MyFactory);
#endif

}
