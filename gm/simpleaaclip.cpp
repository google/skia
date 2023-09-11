/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkAAClip.h"
#include "src/core/SkMask.h"
#include "tools/ToolUtils.h"

namespace skiagm {

static void paint_rgn(SkCanvas* canvas, const SkAAClip& clip,
                      const SkPaint& paint) {
    SkMaskBuilder mask;
    SkBitmap bm;

    clip.copyToMask(&mask);

    SkAutoMaskFreeImage amfi(mask.image());

    bm.installMaskPixels(mask);

    // need to copy for deferred drawing test to work
    SkBitmap bm2;

    ToolUtils::copy_to(&bm2, bm.colorType(), bm);

    canvas->drawImage(bm2.asImage(),
                      SK_Scalar1 * mask.fBounds.fLeft,
                      SK_Scalar1 * mask.fBounds.fTop,
                      SkSamplingOptions(),
                      &paint);
}

//////////////////////////////////////////////////////////////////////////////
/*
 * This GM tests anti aliased single operation booleans with SkAAClips,
 * SkRect and SkPaths.
 */
class SimpleClipGM : public GM {
public:
    enum SkGeomTypes {
        kRect_GeomType,
        kPath_GeomType,
        kAAClip_GeomType
    };

    SimpleClipGM(SkGeomTypes geomType)
    : fGeomType(geomType) {
    }

protected:
    void onOnceBeforeDraw() override {
        // offset the rects a bit so we get anti-aliasing in the rect case
        fBase.setLTRB(100.65f,
                      100.65f,
                      150.65f,
                      150.65f);
        fRect = fBase;
        fRect.inset(5, 5);
        fRect.offset(25, 25);

        fBasePath.addRoundRect(fBase, SkIntToScalar(5), SkIntToScalar(5));
        fRectPath.addRoundRect(fRect, SkIntToScalar(5), SkIntToScalar(5));
        INHERITED::setBGColor(0xFFDDDDDD);
    }

    void buildRgn(SkAAClip* clip, SkClipOp op) {
        clip->setPath(fBasePath, fBasePath.getBounds().roundOut(), true);

        SkAAClip clip2;
        clip2.setPath(fRectPath, fRectPath.getBounds().roundOut(), true);
        clip->op(clip2, op);
    }

    void drawOrig(SkCanvas* canvas) {
        SkPaint     paint;

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);

        canvas->drawRect(fBase, paint);
        canvas->drawRect(fRect, paint);
    }

    void drawRgnOped(SkCanvas* canvas, SkClipOp op, SkColor color) {

        SkAAClip clip;

        this->buildRgn(&clip, op);
        this->drawOrig(canvas);

        SkPaint paint;
        paint.setColor(color);
        paint_rgn(canvas, clip, paint);
    }

    void drawPathsOped(SkCanvas* canvas, SkClipOp op, SkColor color) {

        this->drawOrig(canvas);

        canvas->save();

        // create the clip mask with the supplied boolean op
        if (kPath_GeomType == fGeomType) {
            // path-based case
            canvas->clipPath(fBasePath, true);
            canvas->clipPath(fRectPath, op, true);
        } else {
            // rect-based case
            canvas->clipRect(fBase, true);
            canvas->clipRect(fRect, op, true);
        }

        // draw a rect that will entirely cover the clip mask area
        SkPaint paint;
        paint.setColor(color);

        SkRect r = SkRect::MakeLTRB(SkIntToScalar(90),  SkIntToScalar(90),
                                    SkIntToScalar(180), SkIntToScalar(180));

        canvas->drawRect(r, paint);

        canvas->restore();
    }

    SkString getName() const override {
        SkString str;
        str.printf("simpleaaclip_%s",
                    kRect_GeomType == fGeomType ? "rect" :
                    (kPath_GeomType == fGeomType ? "path" :
                    "aaclip"));
        return str;
    }

    SkISize getISize() override { return SkISize::Make(500, 240); }

    void onDraw(SkCanvas* canvas) override {

        const struct {
            SkColor         fColor;
            const char*     fName;
            SkClipOp        fOp;
        } gOps[] = {
                {SK_ColorBLACK, "Difference", SkClipOp::kDifference},
                {SK_ColorRED, "Intersect", SkClipOp::kIntersect},
        };

        SkPaint textPaint;
        SkFont  font(ToolUtils::create_portable_typeface(), 24);
        int xOff = 0;

        for (size_t op = 0; op < std::size(gOps); op++) {
            canvas->drawString(gOps[op].fName, 75.0f, 50.0f, font, textPaint);

            if (kAAClip_GeomType == fGeomType) {
                this->drawRgnOped(canvas, gOps[op].fOp, gOps[op].fColor);
            } else {
                this->drawPathsOped(canvas, gOps[op].fOp, gOps[op].fColor);
            }

            if (xOff >= 400) {
                canvas->translate(SkIntToScalar(-400), SkIntToScalar(250));
                xOff = 0;
            } else {
                canvas->translate(SkIntToScalar(200), 0);
                xOff += 200;
            }
        }
    }
private:

    SkGeomTypes fGeomType;

    SkRect fBase;
    SkRect fRect;

    SkPath fBasePath;       // fBase as a round rect
    SkPath fRectPath;       // fRect as a round rect

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

// rects
DEF_GM( return new SimpleClipGM(SimpleClipGM::kRect_GeomType); )
DEF_GM( return new SimpleClipGM(SimpleClipGM::kPath_GeomType); )
DEF_GM( return new SimpleClipGM(SimpleClipGM::kAAClip_GeomType); )

}  // namespace skiagm
