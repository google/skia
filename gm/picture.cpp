/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

static sk_sp<SkPicture> make_picture() {
    SkPictureRecorder rec;
    SkCanvas* canvas = rec.beginRecording(100, 100);

    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;

    paint.setColor(0x800000FF);
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);

    paint.setColor(0x80FF0000);
    path.moveTo(0, 0); path.lineTo(100, 0); path.lineTo(100, 100);
    canvas->drawPath(path, paint);

    paint.setColor(0x8000FF00);
    path.reset(); path.moveTo(0, 0); path.lineTo(100, 0); path.lineTo(0, 100);
    canvas->drawPath(path, paint);

    paint.setColor(0x80FFFFFF);
    paint.setBlendMode(SkBlendMode::kPlus);
    canvas->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), paint);

    return rec.finishRecordingAsPicture();
}

// Exercise the optional arguments to drawPicture
//
class PictureGM : public skiagm::GM {
public:
    PictureGM()
        : fPicture(nullptr)
    {}

protected:
    void onOnceBeforeDraw() override {
         fPicture = make_picture();
    }

    SkString onShortName() override {
        return SkString("pictures");
    }

    SkISize onISize() override {
        return SkISize::Make(450, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);

        SkMatrix matrix;
        SkPaint paint;

        canvas->drawPicture(fPicture);

        matrix.setTranslate(110, 0);
        canvas->drawPicture(fPicture, &matrix, nullptr);

        matrix.postTranslate(110, 0);
        canvas->drawPicture(fPicture, &matrix, &paint);

        paint.setAlphaf(0.5f);
        matrix.postTranslate(110, 0);
        canvas->drawPicture(fPicture, &matrix, &paint);
    }

private:
    sk_sp<SkPicture> fPicture;

    typedef skiagm::GM INHERITED;
};

// Exercise drawing a picture with a cull rect of non-zero top-left corner.
//
// See skbug.com/9334, which would fail
// ```
//   dm -m picture_cull_rect --config serialize-8888
// ```
// until that bug is fixed.
class PictureCullRectGM : public skiagm::GM {
public:
    PictureCullRectGM()
        : fPicture(nullptr)
    {}

protected:
    void onOnceBeforeDraw() override {
        SkPictureRecorder rec;
        SkRTreeFactory rtreeFactory;
        SkCanvas* canvas = rec.beginRecording(100, 100, &rtreeFactory);

        SkPaint paint;
        paint.setAntiAlias(false);

        SkRect rect = SkRect::MakeLTRB(0, 80, 100, 100);

        // Make picture complex enough to trigger the cull rect and bbh (RTree) computations.
        // (A single drawRect won't trigger it.)
        paint.setColor(0x800000FF);
        canvas->drawRect(rect, paint);
        canvas->drawOval(rect, paint);

        fPicture = rec.finishRecordingAsPicture();

        SkASSERT(fPicture->cullRect().top() == 80);
    }

    SkString onShortName() override {
        return SkString("picture_cull_rect");
    }

    SkISize onISize() override {
        return SkISize::Make(120, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clipRect(SkRect::MakeLTRB(0, 60, 120, 120));
        canvas->translate(10, 10);
        canvas->drawPicture(fPicture);
    }

private:
    sk_sp<SkPicture> fPicture;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new PictureGM;)
DEF_GM(return new PictureCullRectGM;)
