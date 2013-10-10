/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkTypeface.h"

// GM to stress the GPU font cache

const char* gFamilyNames[] = {
    "sans-serif", "serif", "monospace"
};

const SkTypeface::Style gStyles[] = {
    SkTypeface::kNormal, SkTypeface::kItalic
};

const SkScalar gTextSizes[] = {
    12, 14, 16, 18, 20, 22, 24, 26, 28, 30
};

#define TYPEFACE_COUNT (SK_ARRAY_COUNT(gFamilyNames)*SK_ARRAY_COUNT(gStyles))

static SkScalar draw_string(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawText(text.c_str(), text.size(), x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
}

class FontCacheGM : public skiagm::GM {
public:
    FontCacheGM() {
        for (size_t i = 0; i < TYPEFACE_COUNT; ++i) {
            fTypefaces[i] = NULL;
        }
    }

    virtual ~FontCacheGM() {
        for (size_t i = 0; i < TYPEFACE_COUNT; ++i) {
            SkSafeUnref(fTypefaces[i]);
        }
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("fontcache");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 320);
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        int typefaceCount = 0;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gFamilyNames); ++i) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(gStyles); ++j) {
                fTypefaces[typefaceCount++] = SkTypeface::CreateFromName(gFamilyNames[i],
                                                                          gStyles[j]);
            }
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkScalar y = 32;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);

        SkString text("Ham");

        // draw some initial text to partially fill the GPU cache
        for (size_t i = 0; i < 2; ++i) {
            paint.setTypeface(fTypefaces[i]);
            SkScalar x = 20;

            for (size_t j = 0; j < SK_ARRAY_COUNT(gTextSizes); ++j) {
                paint.setTextSize(gTextSizes[j]);
                x = draw_string(canvas, text, x, y, paint) + 19;
            }
            y += 32;
        }

        // force a flush
        canvas->flush();

        // draw again, and more to overflow the cache
        for (size_t i = 0; i < TYPEFACE_COUNT; ++i) {
            paint.setTypeface(fTypefaces[i]);
            SkScalar x = 20;

            for (size_t j = 0; j < SK_ARRAY_COUNT(gTextSizes); ++j) {
                paint.setTextSize(gTextSizes[j]);
                x = draw_string(canvas, text, x, y, paint) + 19;
            }
            y += 32;
        }

    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // this GM is meant only for the GPU
        return kGPUOnly_Flag;
    }

private:
    SkTypeface* fTypefaces[TYPEFACE_COUNT];
    typedef GM INHERITED;
};


//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(FontCacheGM); )
