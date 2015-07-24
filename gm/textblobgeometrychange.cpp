/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkTextBlob.h"

// This tests that we don't try to reuse textblobs from the GPU textblob cache across pixel geometry
// changes when we have LCD.  crbug/486744
namespace skiagm {
class TextBlobGeometryChange : public GM {
public:
    TextBlobGeometryChange() { }

protected:
    SkString onShortName() override {
        return SkString("textblobgeometrychange");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        const char text[] = "Hamburgefons";

        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(20);
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);

        SkTextBlobBuilder builder;

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 10, 10);

        SkAutoTUnref<const SkTextBlob> blob(builder.build());

        SkImageInfo info = SkImageInfo::MakeN32Premul(200, 200);
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info, &props));
        if (surface) {
            SkCanvas* c = surface->getCanvas();

            // LCD text on white background
            SkRect rect = SkRect::MakeLTRB(0.f, 0.f, SkIntToScalar(kWidth), kHeight / 2.f);
            SkPaint rectPaint;
            rectPaint.setColor(0xffffffff);
            canvas->drawRect(rect, rectPaint);
            canvas->drawTextBlob(blob.get(), 10, 50, paint);

            // This should not look garbled since we should disable LCD text in this case
            // (i.e., unknown pixel geometry)
            c->clear(0x00ffffff);
            c->drawTextBlob(blob.get(), 10, 150, paint);
            surface->draw(canvas, 0, 0, nullptr);
        } else {
            const char* text = "This test requires a surface";
            size_t len = strlen(text);
            SkPaint paint;
            canvas->drawText(text, len, 10, 100, paint);
        }
    }

private:
    static const int kWidth = 200;
    static const int kHeight = 200;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(TextBlobGeometryChange); )
}
