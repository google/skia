#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
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
#if 0
    SkCanvas canvas;
    canvas.setBitmapDevice(bitmap);
#endif

   // set up clipper
   SkRect skclip;
   skclip.set(SkIntToFixed(284), SkIntToFixed(40), SkIntToFixed(1370), SkIntToFixed(708));

//   ret = canvas.clipRect(skclip);
//   SkASSERT(ret);

   matrix.set(SkMatrix::kMTransX, SkFloatToFixed(-1153.28));
   matrix.set(SkMatrix::kMTransY, SkFloatToFixed(1180.50));

   matrix.set(SkMatrix::kMScaleX, SkFloatToFixed(0.177171));
   matrix.set(SkMatrix::kMScaleY, SkFloatToFixed(0.177043));

   matrix.set(SkMatrix::kMSkewX, SkFloatToFixed(0.126968));
   matrix.set(SkMatrix::kMSkewY, SkFloatToFixed(-0.126876));

   matrix.set(SkMatrix::kMPersp0, SkFloatToFixed(0.0));
   matrix.set(SkMatrix::kMPersp1, SkFloatToFixed(0.0));

   ret = canvas.concat(matrix);

   paint.setAntiAlias(true);
   paint.setColor(0xb2202020);
   paint.setStyle(SkPaint::kStroke_Style);
   paint.setStrokeWidth(SkFloatToFixed(68.13));

   SkRect r;
   r.set(SkFloatToFixed(-313.714417), SkFloatToFixed(-4.826389), SkFloatToFixed(18014.447266), SkFloatToFixed(1858.154541));
   canvas.drawRoundRect(r, SkFloatToFixed(91.756363), SkFloatToFixed(91.756363), paint);
}

// ownership of the stream is transferred
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
        
        SkScalar scale = SK_Scalar1 * 999/1000;
//        scale = SK_Scalar1/2;
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
   //     canvas->scale(scale, scale);
        
        SkScalar x = SkIntToScalar(32), y = SkIntToScalar(32);
        SkPaint paint;
        
    //    x += fDX;
    //    y += fDY;
    
//        paint.setLooper(new SkBlurDrawLooper(SkIntToScalar(12), 0, 0, 0xDD000000))->unref();
        
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

