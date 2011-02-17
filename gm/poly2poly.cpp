#include "gm.h"

namespace skiagm {

class Poly2PolyGM : public GM {
public:
	Poly2PolyGM() {}
    
protected:
    virtual SkString onShortName() {
        return SkString("poly2poly");
    }

	virtual SkISize onISize() {
        return make_isize(835, 840);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
	
    static void doDraw(SkCanvas* canvas, SkPaint* paint, const int isrc[],
                       const int idst[], int count) {
        SkMatrix matrix;
        SkPoint src[4], dst[4];
        
        for (int i = 0; i < count; i++) {
            src[i].set(SkIntToScalar(isrc[2*i+0]), SkIntToScalar(isrc[2*i+1]));
            dst[i].set(SkIntToScalar(idst[2*i+0]), SkIntToScalar(idst[2*i+1]));
        }
        
        canvas->save();
        matrix.setPolyToPoly(src, dst, count);
        canvas->concat(matrix);
        
        paint->setColor(SK_ColorGRAY);
        paint->setStyle(SkPaint::kStroke_Style);
        const SkScalar D = SkIntToScalar(64);
        canvas->drawRectCoords(0, 0, D, D, *paint);
        canvas->drawLine(0, 0, D, D, *paint);
        canvas->drawLine(0, D, D, 0, *paint);
        
        SkPaint::FontMetrics fm;
        paint->getFontMetrics(&fm);
        paint->setColor(SK_ColorRED);
        paint->setStyle(SkPaint::kFill_Style);
        SkScalar x = D/2;
        float y = D/2 - (fm.fAscent + fm.fDescent)/2;
        SkString str;
        str.appendS32(count);
        canvas->drawText(str.c_str(), str.size(), x, y, *paint);
        
        canvas->restore();
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(4));
        paint.setTextSize(SkIntToScalar(40));
        paint.setTextAlign(SkPaint::kCenter_Align);
		
        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        // translate (1 point)
        const int src1[] = { 0, 0 };
        const int dst1[] = { 5, 5 };
        doDraw(canvas, &paint, src1, dst1, 1);
        canvas->restore();
        
        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(10));
        // rotate/uniform-scale (2 points)
        const int src2[] = { 32, 32, 64, 32 };
        const int dst2[] = { 32, 32, 64, 48 };
        doDraw(canvas, &paint, src2, dst2, 2);
        canvas->restore();
        
        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(110));
        // rotate/skew (3 points)
        const int src3[] = { 0, 0, 64, 0, 0, 64 };
        const int dst3[] = { 0, 0, 96, 0, 24, 64 };
        doDraw(canvas, &paint, src3, dst3, 3);
        canvas->restore();
        
        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(110));
        // perspective (4 points)
        const int src4[] = { 0, 0, 64, 0, 64, 64, 0, 64 };
        const int dst4[] = { 0, 0, 96, 0, 64, 96, 0, 64 };
        doDraw(canvas, &paint, src4, dst4, 4);
        canvas->restore();
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new Poly2PolyGM; }
static GMRegistry reg(MyFactory);

}

