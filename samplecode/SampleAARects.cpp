#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkShader.h"

static SkBitmap createBitmap(int n) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, n, n);
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorGREEN);
    
    SkCanvas canvas(bitmap);
    SkRect r;
    r.set(0, 0, SkIntToScalar(n), SkIntToScalar(n));
    SkPaint paint;
    paint.setAntiAlias(true);
    
    paint.setColor(SK_ColorRED);
    canvas.drawOval(r, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(SkIntToScalar(n)/15);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.drawLine(0, 0, r.fRight, r.fBottom, paint);
    canvas.drawLine(0, r.fBottom, r.fRight, 0, paint);
    
    return bitmap;
}

class AARectView : public SampleView {
    SkBitmap fBitmap;
    enum {
        N = 64
    };
public:
    AARectView() {
        fBitmap = createBitmap(N);
        
        fWidth = N;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AA Rects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));

        SkPaint bluePaint;
        bluePaint.setARGB(0xff, 0x0, 0x0, 0xff);
        SkPaint bmpPaint;
        SkShader* bmpShader = SkShader::CreateBitmapShader(fBitmap, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
        bmpPaint.setShader(bmpShader);
        bmpShader->unref();

        bluePaint.setStrokeWidth(3);
        bmpPaint.setStrokeWidth(3);

        SkPaint paints[] = { bluePaint, bmpPaint };

        SkRect rect;

        SkScalar dx = SkIntToScalar(80);
        SkScalar dy = SkIntToScalar(100);
        SkMatrix matrix;
        for (size_t p = 0; p < SK_ARRAY_COUNT(paints); ++p) {
            for (int stroke = 0; stroke < 2; ++stroke) {
                paints[p].setStyle(stroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
                for (int a = 0; a < 3; ++ a) {
                    paints[p].setAntiAlias(a > 0);
                    paints[p].setAlpha(a > 1 ? 0x80 : 0xff);

                    canvas->save();
                        rect = SkRect::MakeLTRB(SkFloatToScalar(0.f),
                                                SkFloatToScalar(0.f),
                                                SkFloatToScalar(40.f),
                                                SkFloatToScalar(40.f));
                        canvas->drawRect(rect, paints[p]);
                        canvas->translate(dx, 0);

                        rect = SkRect::MakeLTRB(SkFloatToScalar(0.5f),
                                                SkFloatToScalar(0.5f),
                                                SkFloatToScalar(40.5f),
                                                SkFloatToScalar(40.5f));
                        canvas->drawRect(rect, paints[p]);
                        canvas->translate(dx, 0);

                        rect = SkRect::MakeLTRB(SkFloatToScalar(0.5f),
                                                SkFloatToScalar(0.5f),
                                                SkFloatToScalar(40.f),
                                                SkFloatToScalar(40.f));
                        canvas->drawRect(rect, paints[p]);
                        canvas->translate(dx, 0);

                        rect = SkRect::MakeLTRB(SkFloatToScalar(0.75f),
                                                SkFloatToScalar(0.75f),
                                                SkFloatToScalar(40.75f),
                                                SkFloatToScalar(40.75f));
                        canvas->drawRect(rect, paints[p]);
                        canvas->translate(dx, 0);

                        canvas->save();
                            canvas->translate(SkFloatToScalar(.33f), SkFloatToScalar(.67f));
                            rect = SkRect::MakeLTRB(SkFloatToScalar(0.0f),
                                                    SkFloatToScalar(0.0f),
                                                    SkFloatToScalar(40.0f),
                                                    SkFloatToScalar(40.0f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                        canvas->save();
                            matrix.setRotate(SkFloatToScalar(45.f));
                            canvas->concat(matrix);
                            canvas->translate(SkFloatToScalar(20.0f / sqrtf(2.f)),
                                                SkFloatToScalar(20.0f / sqrtf(2.f)));
                            rect = SkRect::MakeLTRB(SkFloatToScalar(-20.0f),
                                                    SkFloatToScalar(-20.0f),
                                                    SkFloatToScalar(20.0f),
                                                    SkFloatToScalar(20.0f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                        canvas->save();
                            canvas->rotate(SkFloatToScalar(90.f));
                            rect = SkRect::MakeLTRB(SkFloatToScalar(0.0f),
                                                    SkFloatToScalar(0.0f),
                                                    SkFloatToScalar(40.0f),
                                                    SkFloatToScalar(-40.0f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                        canvas->save();
                            canvas->rotate(SkFloatToScalar(90.f));
                            rect = SkRect::MakeLTRB(SkFloatToScalar(0.5f),
                                                    SkFloatToScalar(0.5f),
                                                    SkFloatToScalar(40.5f),
                                                    SkFloatToScalar(-40.5f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                        canvas->save();
                            matrix.setScale(SkFloatToScalar(-1.f), SkFloatToScalar(-1.f));
                            canvas->concat(matrix);
                            rect = SkRect::MakeLTRB(SkFloatToScalar(0.5f),
                                                    SkFloatToScalar(0.5f),
                                                    SkFloatToScalar(-40.5f),
                                                    SkFloatToScalar(-40.5f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                        canvas->save();
                            matrix.setScale(SkFloatToScalar(2.1f), SkFloatToScalar(4.1f));
                            canvas->concat(matrix);
                            rect = SkRect::MakeLTRB(SkFloatToScalar(0.1f),
                                                    SkFloatToScalar(0.1f),
                                                    SkFloatToScalar(19.1f),
                                                    SkFloatToScalar(9.1f));
                            canvas->drawRect(rect, paints[p]);
                        canvas->restore();
                        canvas->translate(dx, 0);

                    canvas->restore();
                    canvas->translate(0, dy);
                }
            }
        }
    }
    
private:
    int fWidth;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AARectView; }
static SkViewRegister reg(MyFactory);

