/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"

///////////////////////////////////////////////////////////////////////////////

class LayerMaskView : public Sample {
public:
    LayerMaskView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkString name() { return SkString("LayerMask"); }

    void drawMask(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setAntiAlias(true);

        if (true) {
            SkBitmap mask;
            int w = SkScalarRoundToInt(r.width());
            int h = SkScalarRoundToInt(r.height());
            mask.allocN32Pixels(w, h);
            mask.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas c(mask);
            SkRect bounds = r;
            bounds.offset(-bounds.fLeft, -bounds.fTop);
            c.drawOval(bounds, paint);

            paint.setBlendMode(SkBlendMode::kDstIn);
            canvas->drawBitmap(mask, r.fLeft, r.fTop, &paint);
        } else {
            SkPath p;
            p.addOval(r);
            p.setFillType(SkPathFillType::kInverseWinding);
            paint.setBlendMode(SkBlendMode::kDstOut);
            canvas->drawPath(p, paint);
        }
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkRect  r;
        r.setLTRB(20, 20, 120, 120);
        canvas->saveLayer(&r, nullptr);
        canvas->drawColor(SK_ColorRED);
        drawMask(canvas, r);
        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new LayerMaskView(); )
