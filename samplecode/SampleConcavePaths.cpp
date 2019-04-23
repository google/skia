/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkParsePath.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

#include "src/core/SkGeometry.h"

class ConcavePathView : public Sample {
public:
    ConcavePathView() {}

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "ConcavePaths");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        // Concave test
        if (1) {
            SkPath path;
            canvas->translate(0, 0);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(30), SkIntToScalar(30));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            canvas->drawPath(path, paint);
        }
        // Reverse concave test
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(100, 0);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(30), SkIntToScalar(30));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
        // Bowtie (intersection)
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(200, 0);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
        // "fake" bowtie (concave, but no intersection)
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(300, 0);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(50), SkIntToScalar(40));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(50), SkIntToScalar(60));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
        // Fish test (intersection/concave)
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(0, 100);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(70), SkIntToScalar(50));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(0), SkIntToScalar(50));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
        // Collinear test
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(100, 100);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(50), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(50), SkIntToScalar(80));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
        // Hole test
        if (1) {
            SkPath path;
            canvas->save();
            canvas->translate(200, 100);
            path.moveTo(SkIntToScalar(20), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(20));
            path.lineTo(SkIntToScalar(80), SkIntToScalar(80));
            path.lineTo(SkIntToScalar(20), SkIntToScalar(80));
            path.moveTo(SkIntToScalar(30), SkIntToScalar(30));
            path.lineTo(SkIntToScalar(30), SkIntToScalar(70));
            path.lineTo(SkIntToScalar(70), SkIntToScalar(70));
            path.lineTo(SkIntToScalar(70), SkIntToScalar(30));
            canvas->drawPath(path, paint);
            canvas->restore();
        }
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ConcavePathView(); )
