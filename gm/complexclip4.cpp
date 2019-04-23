/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "src/core/SkClipOpPriv.h"

namespace skiagm {

//this test exercise SkCanvas::setDeviceClipRestriction behavior
class ComplexClip4GM : public GM {
public:
  ComplexClip4GM(bool aaclip)
    : fDoAAClip(aaclip) {
        this->setBGColor(0xFFDEDFDE);
    }

protected:


    SkString onShortName() {
        SkString str;
        str.printf("complexclip4_%s",
                   fDoAAClip ? "aa" : "bw");
        return str;
    }

    SkISize onISize() { return SkISize::Make(970, 780); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint p;
        p.setAntiAlias(fDoAAClip);
        p.setColor(SK_ColorYELLOW);

        canvas->save();
            // draw a yellow rect through a rect clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(100, 100, 300, 300));
                canvas->drawColor(SK_ColorGREEN);
                canvas->clipRect(SkRect::MakeLTRB(100, 200, 400, 500),
                                 kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(100, 200, 400, 500), p);
            canvas->restore();

            // draw a yellow rect through a diamond clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(500, 100, 800, 300));
                canvas->drawColor(SK_ColorGREEN);

                SkPath pathClip;
                pathClip.moveTo(SkIntToScalar(650),  SkIntToScalar(200));
                pathClip.lineTo(SkIntToScalar(900), SkIntToScalar(300));
                pathClip.lineTo(SkIntToScalar(650), SkIntToScalar(400));
                pathClip.lineTo(SkIntToScalar(650), SkIntToScalar(300));
                pathClip.close();
                canvas->clipPath(pathClip, kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 200, 900, 500), p);
            canvas->restore();

            // draw a yellow rect through a round rect clip
            canvas->save();
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(500, 500, 800, 700));
                canvas->drawColor(SK_ColorGREEN);

                canvas->clipRRect(SkRRect::MakeOval(SkRect::MakeLTRB(500, 600, 900, 750)),
                                  kReplace_SkClipOp, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 600, 900, 750), p);
            canvas->restore();

            // fill the clip with yellow color showing that androidFramework_setDeviceClipRestriction
            // intersects with the current clip
            canvas->save();
                canvas->clipRect(SkRect::MakeLTRB(100, 400, 300, 750),
                                 kIntersect_SkClipOp, fDoAAClip);
                canvas->drawColor(SK_ColorGREEN);
                canvas->androidFramework_setDeviceClipRestriction(SkIRect::MakeLTRB(150, 450, 250, 700));
                canvas->drawColor(SK_ColorYELLOW);
            canvas->restore();

        canvas->restore();
    }
private:

    bool fDoAAClip;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ComplexClip4GM(false);)
DEF_GM(return new ComplexClip4GM(true);)
}
