/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"
#include "src/core/SkClipOpPriv.h"

class ComplexClipView : public Sample {
public:
    ComplexClipView() {
        this->setBGColor(0xFFA0DDA0);
    }

protected:
    virtual SkString name() { return SkString("ComplexClip"); }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPath path;
        path.moveTo(SkIntToScalar(0),   SkIntToScalar(50));
        path.quadTo(SkIntToScalar(0),   SkIntToScalar(0),   SkIntToScalar(50),  SkIntToScalar(0));
        path.lineTo(SkIntToScalar(175), SkIntToScalar(0));
        path.quadTo(SkIntToScalar(200), SkIntToScalar(0),   SkIntToScalar(200), SkIntToScalar(25));
        path.lineTo(SkIntToScalar(200), SkIntToScalar(150));
        path.quadTo(SkIntToScalar(200), SkIntToScalar(200), SkIntToScalar(150), SkIntToScalar(200));
        path.lineTo(SkIntToScalar(0),   SkIntToScalar(200));
        path.close();
        path.moveTo(SkIntToScalar(50),  SkIntToScalar(50));
        path.lineTo(SkIntToScalar(150), SkIntToScalar(50));
        path.lineTo(SkIntToScalar(150), SkIntToScalar(125));
        path.quadTo(SkIntToScalar(150), SkIntToScalar(150), SkIntToScalar(125), SkIntToScalar(150));
        path.lineTo(SkIntToScalar(50),  SkIntToScalar(150));
        path.close();
        path.setFillType(SkPath::kEvenOdd_FillType);
        SkColor pathColor = SK_ColorBLACK;
        SkPaint pathPaint;
        pathPaint.setAntiAlias(true);
        pathPaint.setColor(pathColor);

        SkPath clipA;
        clipA.moveTo(SkIntToScalar(10),  SkIntToScalar(20));
        clipA.lineTo(SkIntToScalar(165), SkIntToScalar(22));
        clipA.lineTo(SkIntToScalar(70),  SkIntToScalar(105));
        clipA.lineTo(SkIntToScalar(165), SkIntToScalar(177));
        clipA.lineTo(SkIntToScalar(-5),  SkIntToScalar(180));
        clipA.close();
        SkColor colorA = SK_ColorCYAN;

        SkPath clipB;
        clipB.moveTo(SkIntToScalar(40),  SkIntToScalar(10));
        clipB.lineTo(SkIntToScalar(190), SkIntToScalar(15));
        clipB.lineTo(SkIntToScalar(195), SkIntToScalar(190));
        clipB.lineTo(SkIntToScalar(40),  SkIntToScalar(185));
        clipB.lineTo(SkIntToScalar(155), SkIntToScalar(100));
        clipB.close();
        SkColor colorB = SK_ColorRED;

        SkFont font;
        font.setSize(20);

        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);

        canvas->translate(SkIntToScalar(10),SkIntToScalar(10));
        canvas->drawPath(path, pathPaint);
        paint.setColor(colorA);
        canvas->drawPath(clipA, paint);
        paint.setColor(colorB);
        canvas->drawPath(clipB, paint);

        static const struct {
            SkClipOp    fOp;
            const char* fName;
        } gOps[] = { //extra spaces in names for measureText
            {kIntersect_SkClipOp,         "Isect "},
            {kDifference_SkClipOp,        "Diff " },
            {kUnion_SkClipOp,             "Union "},
            {kXOR_SkClipOp,               "Xor "  },
            {kReverseDifference_SkClipOp, "RDiff "}
        };

        canvas->translate(0, SkIntToScalar(40));
        canvas->scale(3 * SK_Scalar1 / 4, 3 * SK_Scalar1 / 4);
        canvas->save();

        for (int invA = 0; invA < 2; ++invA) {
            for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); ++op) {
                size_t idx = invA * SK_ARRAY_COUNT(gOps) + op;
                if (!(idx % 3)) {
                    canvas->restore();
                    canvas->translate(0, SkIntToScalar(250));
                    canvas->save();
                }
                canvas->save();
                    // set clip
                    clipA.setFillType(invA ? SkPath::kInverseEvenOdd_FillType :
                                             SkPath::kEvenOdd_FillType);
                    canvas->clipPath(clipA);
                    canvas->clipPath(clipB, gOps[op].fOp);

                    // draw path clipped
                    canvas->drawPath(path, pathPaint);
                canvas->restore();

                // draw path in hairline
                paint.setColor(pathColor);
                canvas->drawPath(path, paint);

                // draw clips in hair line
                paint.setColor(colorA);
                canvas->drawPath(clipA, paint);
                paint.setColor(colorB);
                canvas->drawPath(clipB, paint);

                SkScalar txtX = SkIntToScalar(55);
                paint.setColor(colorA);
                const char* aTxt = invA ? "InverseA " : "A ";
                canvas->drawSimpleText(aTxt, strlen(aTxt), SkTextEncoding::kUTF8, txtX, SkIntToScalar(220), font, paint);
                txtX += font.measureText(aTxt, strlen(aTxt), SkTextEncoding::kUTF8);
                paint.setColor(SK_ColorBLACK);
                canvas->drawSimpleText(gOps[op].fName, strlen(gOps[op].fName), SkTextEncoding::kUTF8,
                                    txtX, 220, font, paint);
                txtX += font.measureText(gOps[op].fName, strlen(gOps[op].fName), SkTextEncoding::kUTF8);
                paint.setColor(colorB);
                canvas->drawSimpleText("B", 1, SkTextEncoding::kUTF8, txtX, 220, font, paint);

                canvas->translate(SkIntToScalar(250),0);
            }
        }
        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ComplexClipView(); )
