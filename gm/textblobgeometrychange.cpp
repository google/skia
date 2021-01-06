/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"

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

        SkFont font(ToolUtils::create_portable_typeface(), 20);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkTextBlobBuilder builder;

        ToolUtils::add_to_text_blob(&builder, text, font, 10, 10);

        sk_sp<SkTextBlob> blob(builder.make());

        SkImageInfo info = SkImageInfo::MakeN32Premul(200, 200);
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        auto           surface = ToolUtils::makeSurface(canvas, info, &props);
        SkCanvas* c = surface->getCanvas();

        // LCD text on white background
        SkRect rect = SkRect::MakeLTRB(0.f, 0.f, SkIntToScalar(kWidth), kHeight / 2.f);
        SkPaint rectPaint;
        rectPaint.setColor(0xffffffff);
        canvas->drawRect(rect, rectPaint);
        canvas->drawTextBlob(blob, 10, 50, SkPaint());

        // This should not look garbled since we should disable LCD text in this case
        // (i.e., unknown pixel geometry)
        c->clear(0x00ffffff);
        c->drawTextBlob(blob, 10, 150, SkPaint());
        surface->draw(canvas, 0, 0);
    }

private:
    static constexpr int kWidth = 200;
    static constexpr int kHeight = 200;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobGeometryChange;)
}  // namespace skiagm
