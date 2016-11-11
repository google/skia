/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

#define WIDTH 60
#define HEIGHT 6

class PathBleedGM : public skiagm::GM {
public:
    PathBleedGM() {}

protected:
    SkString onShortName() override {
        return SkString("pathbleed");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
    SkPath path;
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0,4,60,5));
    path.moveTo(SkIntToScalar(4), SkIntToScalar(4));
    path.lineTo(SkIntToScalar(59), SkIntToScalar(4));
    path.lineTo(SkIntToScalar(59), SkIntToScalar(5));
    path.moveTo(SkIntToScalar(4), SkIntToScalar(5));
    path.lineTo(SkIntToScalar(4), SkIntToScalar(4));
    canvas->drawPath(path, paint);
    canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new PathBleedGM; )
