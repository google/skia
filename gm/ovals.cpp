/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkTArray.h"
#include "SkMatrix.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkBlurDrawLooper.h"
#include "SkRect.h"

namespace skiagm {

class OvalGM : public GM {
public:
    OvalGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:

    SkString onShortName() override {
        return SkString("ovals");
    }

    SkISize onISize() override {
        return SkISize::Make(200, 200);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(100 * SK_Scalar1, 100 * SK_Scalar1);
        SkRect oval = SkRect::MakeLTRB(-20, -30, 20, 30);

        canvas->save();
        SkMatrix mat;
        mat.setScale(SkIntToScalar(4), SkIntToScalar(1));
        // position the oval, and make it at off-integer coords.
        mat.postTranslate(SK_Scalar1 / 4,
                          3 * SK_Scalar1 / 4);
        canvas->concat(mat);

        // AA with stroke and fill style
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStrokeAndFill_Style);
        p.setStrokeWidth(SkIntToScalar(3));
        p.setColor(SK_ColorWHITE);

        canvas->drawOval(oval, p);

        canvas->restore();
    }

private:

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new OvalGM; }
static GMRegistry reg(MyFactory);

}
