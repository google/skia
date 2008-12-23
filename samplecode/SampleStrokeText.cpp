#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

static void lettersToBitmap(SkBitmap* dst, const char chars[],
                            const SkPaint& original, SkBitmap::Config config) {
    SkPath path;
    SkScalar x = 0;
    SkScalar width;
    SkPath p;
    for (int i = 0; i < strlen(chars); i++) {
        original.getTextPath(&chars[i], 1, x, 0, &p);
        path.addPath(p);
        original.getTextWidths(&chars[i], 1, &width);
        x += width;
    }
    SkRect bounds;
    path.computeBounds(&bounds, SkPath::kExact_BoundsType);
    SkScalar sw = -original.getStrokeWidth();
    bounds.inset(sw, sw);
    path.offset(-bounds.fLeft, -bounds.fTop);
    bounds.offset(-bounds.fLeft, -bounds.fTop);
    
    int w = SkScalarRound(bounds.width());
    int h = SkScalarRound(bounds.height());
    SkPaint paint(original);
    SkBitmap src;
    src.setConfig(config, w, h);
    src.allocPixels();
    src.eraseColor(0);
    {
        SkCanvas canvas(src);
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kFill_Style);
        canvas.drawPath(path, paint);
    }
    
    dst->setConfig(config, w, h);
    dst->allocPixels();
    dst->eraseColor(SK_ColorWHITE);
    {
        SkCanvas canvas(*dst);
        paint.setPorterDuffXfermode(SkPorterDuff::kDstATop_Mode);
        canvas.drawBitmap(src, 0, 0, &paint);
        paint.setColor(original.getColor());
        paint.setStyle(SkPaint::kStroke_Style);
        canvas.drawPath(path, paint);
    }
}

static void lettersToBitmap2(SkBitmap* dst, const char chars[],
                            const SkPaint& original, SkBitmap::Config config) {
    SkPath path;
    SkScalar x = 0;
    SkScalar width;
    SkPath p;
    for (int i = 0; i < strlen(chars); i++) {
        original.getTextPath(&chars[i], 1, x, 0, &p);
        path.addPath(p);
        original.getTextWidths(&chars[i], 1, &width);
        x += width;
    }
    SkRect bounds;
    path.computeBounds(&bounds, SkPath::kExact_BoundsType);
    SkScalar sw = -original.getStrokeWidth();
    bounds.inset(sw, sw);
    path.offset(-bounds.fLeft, -bounds.fTop);
    bounds.offset(-bounds.fLeft, -bounds.fTop);
    
    int w = SkScalarRound(bounds.width());
    int h = SkScalarRound(bounds.height());
    SkPaint paint(original);

    paint.setAntiAlias(true);
    paint.setPorterDuffXfermode(SkPorterDuff::kDstATop_Mode);
    paint.setColor(original.getColor());
    paint.setStyle(SkPaint::kStroke_Style);
    
    dst->setConfig(config, w, h);
    dst->allocPixels();
    dst->eraseColor(SK_ColorWHITE);

    SkCanvas canvas(*dst);
    canvas.drawPath(path, paint);
}

class StrokeTextView : public SkView {
    bool fAA;
public:
	StrokeTextView() : fAA(false) {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "StrokeText");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFF333333);
        canvas->drawColor(0xFFCC8844);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkBitmap bm;
        SkPaint paint;
        
        paint.setStrokeWidth(SkIntToScalar(6));
        paint.setTextSize(SkIntToScalar(80));
//        paint.setTypeface(Typeface.DEFAULT_BOLD);
        
        lettersToBitmap(&bm, "Test Case", paint, SkBitmap::kARGB_4444_Config);
        
        canvas->drawBitmap(bm, 0, 0);
    }
    
private:
    
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new StrokeTextView; }
static SkViewRegister reg(MyFactory);

