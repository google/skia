
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkComposeShader.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkImageRef_GlobalPool.h"
#include "SkOSFile.h"
#include "SkStream.h"

#include "SkBlurDrawLooper.h"
#include "SkColorMatrixFilter.h"

static void drawmarshmallow(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkPaint paint;
    SkRect r;
    SkMatrix m;

    SkImageDecoder::DecodeFile("/Users/reed/Downloads/3elfs.jpg", &bitmap);
    if (!bitmap.pixelRef()) {
        return;
    }

    SkShader* s = SkShader::CreateBitmapShader(bitmap,
                                               SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);
    paint.setShader(s)->unref();
    m.setTranslate(SkIntToScalar(250), SkIntToScalar(134));
    s->setLocalMatrix(m);

    r.set(SkIntToScalar(250),
          SkIntToScalar(134),
          SkIntToScalar(250 + 449),
          SkIntToScalar(134 + 701));
    paint.setFlags(2);

    canvas->drawRect(r, paint);
}

static void DrawRoundRect(SkCanvas& canvas) {
   bool ret = false;
   SkPaint  paint;
   SkBitmap bitmap;
   SkMatrix matrix;
   matrix.reset();

   bitmap.setConfig(SkBitmap::kARGB_8888_Config, 1370, 812);
   bitmap.allocPixels();

   // set up clipper
   SkRect skclip;
   skclip.set(SkIntToScalar(284), SkIntToScalar(40), SkIntToScalar(1370), SkIntToScalar(708));

//   ret = canvas.clipRect(skclip);
//   SkASSERT(ret);

   matrix.set(SkMatrix::kMTransX, SkFloatToScalar(-1153.28f));
   matrix.set(SkMatrix::kMTransY, SkFloatToScalar(1180.50f));

   matrix.set(SkMatrix::kMScaleX, SkFloatToScalar(0.177171f));
   matrix.set(SkMatrix::kMScaleY, SkFloatToScalar(0.177043f));

   matrix.set(SkMatrix::kMSkewX, SkFloatToScalar(0.126968f));
   matrix.set(SkMatrix::kMSkewY, SkFloatToScalar(-0.126876f));

   matrix.set(SkMatrix::kMPersp0, SkFloatToScalar(0.0f));
   matrix.set(SkMatrix::kMPersp1, SkFloatToScalar(0.0f));

   ret = canvas.concat(matrix);

   paint.setAntiAlias(true);
   paint.setColor(0xb2202020);
   paint.setStyle(SkPaint::kStroke_Style);
   paint.setStrokeWidth(SkFloatToScalar(68.13f));

   SkRect r;
   r.set(SkFloatToScalar(-313.714417f), SkFloatToScalar(-4.826389f), SkFloatToScalar(18014.447266f), SkFloatToScalar(1858.154541f));
   canvas.drawRoundRect(r, SkFloatToScalar(91.756363f), SkFloatToScalar(91.756363f), paint);
}

static bool SetImageRef(SkBitmap* bitmap, SkStream* stream,
                        SkBitmap::Config pref, const char name[] = NULL) {
#if 0
    // test buffer streams
    SkStream* str = new SkBufferStream(stream, 717);
    stream->unref();
    stream = str;
#endif

    SkImageRef* ref = new SkImageRef_GlobalPool(stream, pref, 1);
    ref->setURI(name);
    if (!ref->getInfo(bitmap)) {
        delete ref;
        return false;
    }
    bitmap->setPixelRef(ref)->unref();
    return true;
}

//#define SPECIFIC_IMAGE  "/skimages/72.jpg"
#define SPECIFIC_IMAGE  "/Users/reed/Downloads/3elfs.jpg"

#define IMAGE_DIR       "/skimages/"
#define IMAGE_SUFFIX    ".gif"

class ImageDirView : public SkView {
public:
    SkBitmap*   fBitmaps;
    SkString*   fStrings;
    int         fBitmapCount;
    int         fCurrIndex;
    SkScalar    fSaturation;
    SkScalar    fAngle;

    ImageDirView() {
        SkImageRef_GlobalPool::SetRAMBudget(320 * 1024);

#ifdef SPECIFIC_IMAGE
        fBitmaps = new SkBitmap[3];
        fStrings = new SkString[3];
        fBitmapCount = 3;
        const SkBitmap::Config configs[] = {
            SkBitmap::kARGB_8888_Config,
            SkBitmap::kRGB_565_Config,
            SkBitmap::kARGB_4444_Config
        };
        for (int i = 0; i < fBitmapCount; i++) {
#if 1
            SkStream* stream = new SkFILEStream(SPECIFIC_IMAGE);
            SetImageRef(&fBitmaps[i], stream, configs[i], SPECIFIC_IMAGE);
            stream->unref();
#else
            SkImageDecoder::DecodeFile(SPECIFIC_IMAGE, &fBitmaps[i]);
#endif
        }
#else
        int i, N = 0;
        SkOSFile::Iter  iter(IMAGE_DIR, IMAGE_SUFFIX);
        SkString    name;
        while (iter.next(&name)) {
            N += 1;
        }
        fBitmaps = new SkBitmap[N];
        fStrings = new SkString[N];
        iter.reset(IMAGE_DIR, IMAGE_SUFFIX);
        for (i = 0; i < N; i++) {
            iter.next(&name);
            SkString path(IMAGE_DIR);
            path.append(name);
            SkStream* stream = new SkFILEStream(path.c_str());

            SetImageRef(&fBitmaps[i], stream, SkBitmap::kNo_Config,
                        name.c_str());
            stream->unref();
            fStrings[i] = name;
        }
        fBitmapCount = N;
#endif
        fCurrIndex = 0;
        fDX = fDY = 0;

        fSaturation = SK_Scalar1;
        fAngle = 0;

        fScale = SK_Scalar1;
    }

    virtual ~ImageDirView() {
        delete[] fBitmaps;
        delete[] fStrings;

        SkImageRef_GlobalPool::DumpPool();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("ImageDir: ");
#ifdef SPECIFIC_IMAGE
            str.append(SPECIFIC_IMAGE);
#else
            str.append(IMAGE_DIR);
#endif
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorGRAY);
        canvas->drawColor(SK_ColorWHITE);
    }

    SkScalar fScale;
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        if (true) {
            canvas->scale(SkIntToScalar(2), SkIntToScalar(2));
            drawmarshmallow(canvas);
            return;
        }

        if (false) {
            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SkIntToScalar(4));
            canvas->drawCircle(SkIntToScalar(100), SkIntToScalar(100), SkIntToScalar(50), p);
            p.setAntiAlias(true);
            canvas->drawCircle(SkIntToScalar(300), SkIntToScalar(100), SkIntToScalar(50), p);
        }
        if (false) {
            SkScalar cx = this->width()/2;
            SkScalar cy = this->height()/2;
            canvas->translate(cx, cy);
            canvas->scale(fScale, fScale);
            canvas->translate(-cx, -cy);
            DrawRoundRect(*canvas);
            return;
        }

        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));

        SkScalar x = SkIntToScalar(32), y = SkIntToScalar(32);
        SkPaint paint;

#if 0
        for (int i = 0; i < fBitmapCount; i++) {
            SkPaint p;

#if 1
            const SkScalar cm[] = {
                SkIntToScalar(2), 0, 0, 0, SkIntToScalar(-255),
                0, SkIntToScalar(2), 0, 0, SkIntToScalar(-255),
                0, 0, SkIntToScalar(2), 0, SkIntToScalar(-255),
                0, 0, 0, SkIntToScalar(1), 0
            };
            SkColorFilter* cf = new SkColorMatrixFilter(cm);
            p.setColorFilter(cf)->unref();
#endif

            canvas->drawBitmap(fBitmaps[i], x, y, &p);
            x += SkIntToScalar(fBitmaps[i].width() + 10);
        }
        return;
#endif

        canvas->drawBitmap(fBitmaps[fCurrIndex], x, y, &paint);
#ifndef SPECIFIC_IMAGE
        if (true) {
            fCurrIndex += 1;
            if (fCurrIndex >= fBitmapCount) {
                fCurrIndex = 0;
            }
            this->inval(NULL);
        }
#endif
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        if (true) {
            fCurrIndex += 1;
            if (fCurrIndex >= fBitmapCount)
                fCurrIndex = 0;
            this->inval(NULL);
        }
        return new Click(this);
    }

    virtual bool onClick(Click* click)  {
        SkScalar center = this->width()/2;
        fSaturation = SkScalarDiv(click->fCurr.fX - center, center/2);
        center = this->height()/2;
        fAngle = SkScalarDiv(click->fCurr.fY - center, center) * 180;

        fDX += click->fCurr.fX - click->fPrev.fX;
        fDY += click->fCurr.fY - click->fPrev.fY;

        fScale = SkScalarDiv(click->fCurr.fX, this->width());

        this->inval(NULL);
        return true;
        return this->INHERITED::onClick(click);
    }

private:
    SkScalar fDX, fDY;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ImageDirView; }
static SkViewRegister reg(MyFactory);

