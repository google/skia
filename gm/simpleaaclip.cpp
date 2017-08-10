/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkAAClip.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace skiagm {

static void paint_rgn(SkCanvas* canvas, const SkAAClip& clip,
                      const SkPaint& paint) {
    SkMask mask;
    SkBitmap bm;

    clip.copyToMask(&mask);

    SkAutoMaskFreeImage amfi(mask.fImage);

    bm.installMaskPixels(mask);

    // need to copy for deferred drawing test to work
    SkBitmap bm2;

    sk_tool_utils::copy_to(&bm2, bm.colorType(), bm);

    canvas->drawBitmap(bm2,
                       SK_Scalar1 * mask.fBounds.fLeft,
                       SK_Scalar1 * mask.fBounds.fTop,
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
        fBase.set(100.65f,
                  100.65f,
                  150.65f,
                  150.65f);
        fRect = fBase;
        fRect.inset(5, 5);
        fRect.offset(25, 25);

        fBasePath.addRoundRect(fBase, SkIntToScalar(5), SkIntToScalar(5));
        fRectPath.addRoundRect(fRect, SkIntToScalar(5), SkIntToScalar(5));
        INHERITED::setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

    void buildRgn(SkAAClip* clip, SkClipOp op) {
        clip->setPath(fBasePath, nullptr, true);

        SkAAClip clip2;
        clip2.setPath(fRectPath, nullptr, true);
        clip->op(clip2, (SkRegion::Op)op);
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

    SkString onShortName() override {
        SkString str;
        str.printf("simpleaaclip_%s",
                    kRect_GeomType == fGeomType ? "rect" :
                    (kPath_GeomType == fGeomType ? "path" :
                    "aaclip"));
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {

        const struct {
            SkColor         fColor;
            const char*     fName;
            SkClipOp        fOp;
        } gOps[] = {
            { SK_ColorBLACK,    "Difference", kDifference_SkClipOp    },
            { SK_ColorRED,      "Intersect",  kIntersect_SkClipOp     },
            { sk_tool_utils::color_to_565(0xFF008800), "Union", kUnion_SkClipOp },
            { SK_ColorGREEN,    "Rev Diff",   kReverseDifference_SkClipOp },
            { SK_ColorYELLOW,   "Replace",    kReplace_SkClipOp       },
            { SK_ColorBLUE,     "XOR",        kXOR_SkClipOp           },
        };

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textPaint);
        textPaint.setTextSize(SK_Scalar1*24);
        int xOff = 0;

        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); op++) {
            canvas->drawString(gOps[op].fName,
                             SkIntToScalar(75), SkIntToScalar(50),
                             textPaint);

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

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

// rects
DEF_GM( return new SimpleClipGM(SimpleClipGM::kRect_GeomType); )
DEF_GM( return new SimpleClipGM(SimpleClipGM::kPath_GeomType); )
DEF_GM( return new SimpleClipGM(SimpleClipGM::kAAClip_GeomType); )

}
