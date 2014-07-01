/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkDumpCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkView.h"
#include "SkXMLParser.h"
#include "SkXfermode.h"

///////////////////////////////////////////////////////////////////////////////

static SkBitmap load_bitmap() {
    SkBitmap bm;
    SkString pngFilename = GetResourcePath("mandrill_512.png");
    SkAutoDataUnref data(SkData::NewFromFileName(pngFilename.c_str()));
    if (data.get() != NULL) {
        SkInstallDiscardablePixelRef(SkDecodingImageGenerator::Create(
            data, SkDecodingImageGenerator::Options()), &bm);
    }
    return bm;
}

static void drawCircle(SkCanvas* canvas, int r, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);

    canvas->drawCircle(SkIntToScalar(r), SkIntToScalar(r), SkIntToScalar(r),
                       paint);
}

class PictureView : public SampleView {
    SkBitmap fBitmap;
public:
    PictureView() {

        fBitmap = load_bitmap();

        SkPictureRecorder recorder;

        recorder.beginRecording(100, 100, NULL, 0);
        fSubPicture = recorder.endRecording();

        SkCanvas* canvas = recorder.beginRecording(100, 100, NULL, 0);
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->drawBitmap(fBitmap, 0, 0, NULL);

        drawCircle(canvas, 50, SK_ColorBLACK);
        canvas->drawPicture(fSubPicture);
        canvas->translate(SkIntToScalar(50), 0);
        canvas->drawPicture(fSubPicture);
        canvas->translate(0, SkIntToScalar(50));
        canvas->drawPicture(fSubPicture);
        canvas->translate(SkIntToScalar(-50), 0);
        canvas->drawPicture(fSubPicture);

        fPicture = recorder.endRecording();

        // fPicture now has (4) references to fSubPicture. We can release our ref,
        // and just unref fPicture in our destructor, and it will in turn take care of
        // the other references to fSubPicture
        fSubPicture->unref();
    }

    virtual ~PictureView() {
        fPicture->unref();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Picture");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawSomething(SkCanvas* canvas) {
        SkPaint paint;

        canvas->save();
        canvas->scale(0.5f, 0.5f);
        canvas->drawBitmap(fBitmap, 0, 0, NULL);
        canvas->restore();

        const char beforeStr[] = "before circle";
        const char afterStr[] = "after circle";

        paint.setAntiAlias(true);

        paint.setColor(SK_ColorRED);
        canvas->drawData(beforeStr, sizeof(beforeStr));
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(40), paint);
        canvas->drawData(afterStr, sizeof(afterStr));
        paint.setColor(SK_ColorBLACK);
        paint.setTextSize(SkIntToScalar(40));
        canvas->drawText("Picture", 7, SkIntToScalar(50), SkIntToScalar(62),
                         paint);

    }

    virtual void onDrawContent(SkCanvas* canvas) {
        this->drawSomething(canvas);

        SkPictureRecorder recorder;
        this->drawSomething(recorder.beginRecording(100, 100, NULL, 0));
        SkAutoTUnref<SkPicture> pict(recorder.endRecording());

        canvas->save();
        canvas->translate(SkIntToScalar(300), SkIntToScalar(50));
        canvas->scale(-SK_Scalar1, -SK_Scalar1);
        canvas->translate(-SkIntToScalar(100), -SkIntToScalar(50));
        canvas->drawPicture(pict);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(200), SkIntToScalar(150));
        canvas->scale(SK_Scalar1, -SK_Scalar1);
        canvas->translate(0, -SkIntToScalar(50));
        canvas->drawPicture(pict);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        canvas->scale(-SK_Scalar1, SK_Scalar1);
        canvas->translate(-SkIntToScalar(100), 0);
        canvas->drawPicture(pict);
        canvas->restore();

#ifdef SK_DEVELOPER
        if (false) {
            SkDebugfDumper dumper;
            SkDumpCanvas dumpCanvas(&dumper);
            dumpCanvas.drawPicture(pict);
        }
#endif

        // This used to re-record the sub-picture and redraw the parent
        // A capability that is now forbidden!

        SkRandom rand(SampleCode::GetAnimTime());
        canvas->translate(SkIntToScalar(10), SkIntToScalar(250));
        canvas->drawPicture(fPicture);
        delayInval(500);
    }

private:
    #define INVAL_ALL_TYPE  "inval-all"

    void delayInval(SkMSec delay) {
        (new SkEvent(INVAL_ALL_TYPE, this->getSinkID()))->postDelay(delay);
    }

    virtual bool onEvent(const SkEvent& evt) {
        if (evt.isType(INVAL_ALL_TYPE)) {
            this->inval(NULL);
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    SkPicture*  fPicture;
    SkPicture*  fSubPicture;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PictureView; }
static SkViewRegister reg(MyFactory);
