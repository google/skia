
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkView.h"

/**
 * Animated sample used to develop batched rect implementation in GrBufferedDrawTarget.
 */
class ManyRectsView : public SampleView {
private:
    enum {
        N = 1000,
    };

public:
    ManyRectsView() {}

protected:
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ManyRects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkISize dsize = canvas->getDeviceSize();
        canvas->clear(0xFFF0E0F0);

        for (int i = 0; i < N; ++i) {
            SkRect rect = SkRect::MakeWH(SkIntToScalar(fRandom.nextRangeU(10, 100)),
                                         SkIntToScalar(fRandom.nextRangeU(10, 100)));
            int x = fRandom.nextRangeU(0, dsize.fWidth);
            int y = fRandom.nextRangeU(0, dsize.fHeight);
            canvas->save();

            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            // Rotation messes up the GPU batching because of the clip below. We don't notice
            // that the rect is inside the clip so the clip changes interrupt batching.
            if (false) {
                SkMatrix rotate;
                rotate.setRotate(fRandom.nextUScalar1() * 360,
                                 SkIntToScalar(x) + SkScalarHalf(rect.fRight),
                                 SkIntToScalar(y) + SkScalarHalf(rect.fBottom));
                canvas->concat(rotate);
            }
            SkRect clipRect = rect;
            // This clip will always contain the entire rect. It's here to give the GPU batching
            // code a little more challenge.
            clipRect.outset(10, 10);
            canvas->clipRect(clipRect);
            SkPaint paint;
            paint.setColor(fRandom.nextU());
            canvas->drawRect(rect, paint);
            canvas->restore();
        }
        this->inval(NULL);
    }

private:
    SkRandom fRandom;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ManyRectsView; }
static SkViewRegister reg(MyFactory);
