/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "include/utils/SkRandom.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "src/core/SkClipOpPriv.h"
#include "tools/Resources.h"

// Intended to exercise pixel snapping observed with scaled images (and
// with non-scaled images, but for a different reason):  Bug 1145

class IdentityScaleView : public Sample {
public:
    IdentityScaleView(const char imageFilename[]) {
        if (!DecodeDataToBitmap(GetResourceAsData(imageFilename), &fBM)) {
            fBM.allocN32Pixels(1, 1);
            *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
        }
    }

protected:
    SkBitmap fBM;

    SkString name() override { return SkString("IdentityScale"); }

    void onDrawContent(SkCanvas* canvas) override {

        SkFont font(nullptr, 48);
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setFilterQuality(kHigh_SkFilterQuality);

        SkTime::DateTime time;
        SkTime::GetDateTime(&time);

        bool use_scale = (time.fSecond % 2 == 1);
        const char *text;

        canvas->save();
        if (use_scale) {
          text = "Scaled = 1";
        } else {

          SkRect r = { 100, 100, 356, 356 };
          SkPath clipPath;
          clipPath.addRoundRect(r, SkIntToScalar(5), SkIntToScalar(5));
          canvas->clipPath(clipPath, kIntersect_SkClipOp, true);
          text = "Scaled = 0";
        }
        canvas->drawBitmap( fBM, 100, 100, &paint );
        canvas->restore();
        canvas->drawString(text, 100, 400, font, paint);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new IdentityScaleView("images/mandrill_256.png"); )
