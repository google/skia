/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkCanvasPriv.h"

namespace skiagm {

// This test exercise SkCanvas::androidFramework_replaceClip behavior
class ComplexClip4GM : public GM {
public:
  ComplexClip4GM(bool aaclip)
    : fDoAAClip(aaclip) {
        this->setBGColor(0xFFDEDFDE);
    }

protected:
    SkString onShortName() override {
        SkString str;
        str.printf("complexclip4_%s",
                   fDoAAClip ? "aa" : "bw");
        return str;
    }

    SkISize onISize() override { return SkISize::Make(970, 780); }

    // Android Framework will still support the legacy kReplace SkClipOp on older devices, so
    // this represents how to do so while also respecting the device restriction using the newer
    // androidFramework_resetClip() API.
    void emulateDeviceRestriction(SkCanvas* canvas, const SkIRect& deviceRestriction) {
        // TODO(michaelludwig): It may make more sense for device clip restriction to move on to
        // the SkSurface (which would let this GM draw correctly in viewer).
        canvas->androidFramework_setDeviceClipRestriction(deviceRestriction);
    }

    void emulateClipRectReplace(SkCanvas* canvas,
                                const SkRect& clipRect,
                                bool aa) {
        SkCanvasPriv::ResetClip(canvas);
        canvas->clipRect(clipRect, SkClipOp::kIntersect, aa);
    }

    void emulateClipRRectReplace(SkCanvas* canvas,
                                 const SkRRect& clipRRect,
                                 bool aa) {
        SkCanvasPriv::ResetClip(canvas);
        canvas->clipRRect(clipRRect, SkClipOp::kIntersect, aa);
    }

    void emulateClipPathReplace(SkCanvas* canvas,
                                const SkPath& path,
                                bool aa) {
        SkCanvasPriv::ResetClip(canvas);
        canvas->clipPath(path, SkClipOp::kIntersect, aa);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(fDoAAClip);
        p.setColor(SK_ColorYELLOW);

        canvas->save();
            // draw a yellow rect through a rect clip
            canvas->save();
                emulateDeviceRestriction(canvas, SkIRect::MakeLTRB(100, 100, 300, 300));
                canvas->drawColor(SK_ColorGREEN);
                emulateClipRectReplace(canvas, SkRect::MakeLTRB(100, 200, 400, 500), fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(100, 200, 400, 500), p);
            canvas->restore();

            // draw a yellow rect through a diamond clip
            canvas->save();
                emulateDeviceRestriction(canvas, SkIRect::MakeLTRB(500, 100, 800, 300));
                canvas->drawColor(SK_ColorGREEN);

                SkPath pathClip = SkPath::Polygon({
                    {650, 200},
                    {900, 300},
                    {650, 400},
                    {650, 300},
                }, true);
                emulateClipPathReplace(canvas, pathClip, fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 200, 900, 500), p);
            canvas->restore();

            // draw a yellow rect through a round rect clip
            canvas->save();
                emulateDeviceRestriction(canvas, SkIRect::MakeLTRB(500, 500, 800, 700));
                canvas->drawColor(SK_ColorGREEN);

                emulateClipRRectReplace(
                        canvas, SkRRect::MakeOval(SkRect::MakeLTRB(500, 600, 900, 750)), fDoAAClip);
                canvas->drawRect(SkRect::MakeLTRB(500, 600, 900, 750), p);
            canvas->restore();

            // fill the clip with yellow color showing that androidFramework_replaceClip is
            // in device space
            canvas->save();
                canvas->clipRect(SkRect::MakeLTRB(100, 400, 300, 750),
                                 SkClipOp::kIntersect, fDoAAClip);
                canvas->drawColor(SK_ColorGREEN);
                // should not affect the device-space clip
                canvas->rotate(20.f);
                canvas->translate(50.f, 50.f);
                emulateDeviceRestriction(canvas, SkIRect::MakeLTRB(150, 450, 250, 700));
                canvas->drawColor(SK_ColorYELLOW);
            canvas->restore();

        canvas->restore();
    }
private:
    bool    fDoAAClip;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ComplexClip4GM(false);)
DEF_GM(return new ComplexClip4GM(true);)
}  // namespace skiagm
