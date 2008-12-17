#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"

static void DrawRoundRect() {
#ifdef SK_SCALAR_IS_FIXED
    bool ret = false;
    SkPaint  paint;
    SkBitmap bitmap;
    SkCanvas canvas;
    SkMatrix matrix;
    matrix.reset();
    
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 1370, 812);
    bitmap.allocPixels();
    canvas.setBitmapDevice(bitmap);
    
    // set up clipper
    SkRect skclip;
    skclip.set(SkIntToFixed(284), SkIntToFixed(40), SkIntToFixed(1370), SkIntToFixed(708));
    
    ret = canvas.clipRect(skclip);
    SkASSERT(ret);
    
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
#endif
}

static bool HitTestPath(const SkPath& path, SkScalar x, SkScalar y) {
    SkRegion    rgn, clip;
    
    int ix = SkScalarFloor(x);
    int iy = SkScalarFloor(y);

    clip.setRect(ix, iy, ix + 1, iy + 1);
    
    bool contains = rgn.setPath(path, clip);
    return contains;
}

static void TestOverflowHitTest() {
    SkPath path;
    
#ifdef SK_SCALAR_IS_FLOATx
    path.addCircle(0, 0, 70000, SkPath::kCCW_Direction);
    SkASSERT(HitTestPath(path, 40000, 40000));
#endif
}

class OverflowView : public SkView {
public:
	OverflowView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Circles");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        DrawRoundRect();
        TestOverflowHitTest();
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new OverflowView; }
static SkViewRegister reg(MyFactory);

