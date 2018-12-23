/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "GrContext.h"

// This tests that we correctly regenerate textblobs after freeing all gpu resources crbug/491350
namespace skiagm {
class TextBlobUseAfterGpuFree : public GM {
public:
    TextBlobUseAfterGpuFree() { }

protected:
    SkString onShortName() override {
        return SkString("textblobuseaftergpufree");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        // This GM exists to test a specific feature of the GPU backend.
        if (nullptr == canvas->getGrContext()) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        const char text[] = "Hamburgefons";

        SkFont font(sk_tool_utils::create_portable_typeface(), 20);
        auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

        // draw textblob
        SkRect rect = SkRect::MakeLTRB(0.f, 0.f, SkIntToScalar(kWidth), kHeight / 2.f);
        SkPaint rectPaint;
        rectPaint.setColor(0xffffffff);
        canvas->drawRect(rect, rectPaint);
        canvas->drawTextBlob(blob, 20, 60, SkPaint());

        // This text should look fine
        canvas->getGrContext()->freeGpuResources();
        canvas->drawTextBlob(blob, 20, 160, SkPaint());
    }

private:
    static constexpr int kWidth = 200;
    static constexpr int kHeight = 200;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobUseAfterGpuFree;)
}
