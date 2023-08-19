/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrDirectContext.h"
#include "tools/ToolUtils.h"

#include <string.h>

// This tests that we correctly regenerate textblobs after freeing all gpu resources crbug/491350
namespace skiagm {
class TextBlobUseAfterGpuFree : public GM {
public:
    TextBlobUseAfterGpuFree() { }

protected:
    SkString getName() const override { return SkString("textblobuseaftergpufree"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());

        const char text[] = "Hamburgefons";

        SkFont font(ToolUtils::create_portable_typeface(), 20);
        auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

        // draw textblob
        SkRect rect = SkRect::MakeLTRB(0.f, 0.f, SkIntToScalar(kWidth), kHeight / 2.f);
        SkPaint rectPaint;
        rectPaint.setColor(0xffffffff);
        canvas->drawRect(rect, rectPaint);
        canvas->drawTextBlob(blob, 20, 60, SkPaint());

        // This text should look fine
        if (dContext) {
            dContext->freeGpuResources();
        }
        canvas->drawTextBlob(blob, 20, 160, SkPaint());
    }

private:
    inline static constexpr int kWidth = 200;
    inline static constexpr int kHeight = 200;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobUseAfterGpuFree;)
}  // namespace skiagm
