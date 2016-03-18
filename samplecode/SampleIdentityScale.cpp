/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "gm.h"

#include "Resources.h"
#include "SampleCode.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkTime.h"

// Intended to exercise pixel snapping observed with scaled images (and
// with non-scaled images, but for a different reason):  Bug 1145

class IdentityScaleView : public SampleView {
public:
    IdentityScaleView(const char imageFilename[]) {
      SkString resourcePath = GetResourcePath(imageFilename);
      if (!decode_file(resourcePath.c_str(), &fBM)) {
          fBM.allocN32Pixels(1, 1);
          *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
      }
    }

protected:
    SkBitmap fBM;

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "IdentityScale");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {

        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setTextSize(48);
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
          canvas->clipPath(clipPath, SkRegion::kIntersect_Op, SkToBool(1));
          text = "Scaled = 0";
        }
        canvas->drawBitmap( fBM, 100, 100, &paint );
        canvas->restore();
        canvas->drawText( text, strlen(text), 100, 400, paint );
        this->inval(nullptr);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new IdentityScaleView("mandrill_256.png"); }
static SkViewRegister reg(MyFactory);
