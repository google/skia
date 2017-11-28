/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurImageFilter.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "Resources.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(512, 512);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect r = SkRect::MakeXYWH(128, 128, 256, 256);
        const SkRRect rr = SkRRect::MakeRectXY(r, 128, 128);

        sk_sp<SkImage> image(GetResourceAsImage("mandrill_512.png"));

        canvas->drawImage(image, 0, 0);

        canvas->save();
            canvas->clipRRect(rr, true);

            sk_sp<SkImageFilter> filter = SkBlurImageFilter::Make(10, 10, nullptr);
            SkPaint p;
            p.setImageFilter(std::move(filter));

            SkCanvas::SaveLayerRec slr(nullptr, &p, SkCanvas::kInitWithPrevious_SaveLayerFlag);
            canvas->saveLayer(slr);
                canvas->drawColor(0x40FFFFFF);
            canvas->restore();
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
