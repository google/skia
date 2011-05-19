#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"

#ifdef SK_BUILD_FOR_WIN
// windows doesn't have roundf
inline float roundf(float x) { return (x-floor(x))>0.5 ? ceil(x) : floor(x); }
#endif

#ifdef SK_DEBUG
static void make_rgn(SkRegion* rgn, int left, int top, int right, int bottom,
                     size_t count, int32_t runs[]) {
    SkIRect r;
    r.set(left, top, right, bottom);
    
    rgn->debugSetRuns(runs, count);
    SkASSERT(rgn->getBounds() == r);
}

static void test_union_bug_1505668(SkRegion* ra, SkRegion* rb, SkRegion* rc) {
    static int32_t dataA[] = {
        0x00000001, 0x000001dd,
        0x00000001, 0x0000000c, 0x0000000d, 0x00000025,
        0x7fffffff, 0x000001de, 0x00000001, 0x00000025,
        0x7fffffff, 0x000004b3, 0x00000001, 0x00000026,
        0x7fffffff, 0x000004b4, 0x0000000c, 0x00000026,
        0x7fffffff, 0x00000579, 0x00000000, 0x0000013a,
        0x7fffffff, 0x000005d8, 0x00000000, 0x0000013b,
        0x7fffffff, 0x7fffffff
    };
    make_rgn(ra, 0, 1, 315, 1496, SK_ARRAY_COUNT(dataA), dataA);

    static int32_t dataB[] = {
        0x000000b6, 0x000000c4,
        0x000000a1, 0x000000f0, 0x7fffffff, 0x000000d6,
        0x7fffffff, 0x000000e4, 0x00000070, 0x00000079,
        0x000000a1, 0x000000b0, 0x7fffffff, 0x000000e6,
        0x7fffffff, 0x000000f4, 0x00000070, 0x00000079,
        0x000000a1, 0x000000b0, 0x7fffffff, 0x000000f6,
        0x7fffffff, 0x00000104, 0x000000a1, 0x000000b0,
        0x7fffffff, 0x7fffffff
    };
    make_rgn(rb, 112, 182, 240, 260, SK_ARRAY_COUNT(dataB), dataB);
    
    rc->op(*ra, *rb, SkRegion::kUnion_Op);
}
#endif

static void scale_rect(SkIRect* dst, const SkIRect& src, float scale) {
    dst->fLeft = (int)::roundf(src.fLeft * scale);
    dst->fTop = (int)::roundf(src.fTop * scale);
    dst->fRight = (int)::roundf(src.fRight * scale);
    dst->fBottom = (int)::roundf(src.fBottom * scale);
}

static void scale_rgn(SkRegion* dst, const SkRegion& src, float scale) {
    SkRegion tmp;
    SkRegion::Iterator iter(src);

    for (; !iter.done(); iter.next()) {
        SkIRect r;
        scale_rect(&r, iter.rect(), scale);
        tmp.op(r, SkRegion::kUnion_Op);
    }
    dst->swap(tmp);
}

static void paint_rgn(SkCanvas* canvas, const SkRegion& rgn,
                      const SkPaint& paint) {
    SkRegion scaled;
    scale_rgn(&scaled, rgn, 0.5f);
    
    SkRegion::Iterator  iter(rgn);

    for (; !iter.done(); iter.next())
    {
        SkRect    r;
        r.set(iter.rect());
        canvas->drawRect(r, paint);
    }
}

class RegionView : public SampleView {
public:
	RegionView() {
        fBase.set(100, 100, 150, 150);
        fRect = fBase;
        fRect.inset(5, 5);
        fRect.offset(25, 25);
        this->setBGColor(0xFFDDDDDD);
    }

    void build_rgn(SkRegion* rgn, SkRegion::Op op) {
        rgn->setRect(fBase);
        SkIRect r = fBase;
        r.offset(75, 20);
        rgn->op(r, SkRegion::kUnion_Op);
        rgn->op(fRect, op);
    }


protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Regions");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawOrig(SkCanvas* canvas, bool bg) {
        SkRect      r;
        SkPaint     paint;
        
        paint.setStyle(SkPaint::kStroke_Style);
        if (bg)
            paint.setColor(0xFFBBBBBB);
        
        r.set(fBase);
        canvas->drawRect(r, paint);
        r.set(fRect);
        canvas->drawRect(r, paint);
    }
    
    void drawRgnOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {
        SkRegion    rgn;

        this->build_rgn(&rgn, op);
        
        {
            SkRegion tmp, tmp2(rgn);
            
            tmp = tmp2;
            tmp.translate(5, -3);
            
            {
                char    buffer[1000];
                size_t  size = tmp.flatten(NULL);
                SkASSERT(size <= sizeof(buffer));
                size_t  size2 = tmp.flatten(buffer);
                SkASSERT(size == size2);
                
                SkRegion    tmp3;
                size2 = tmp3.unflatten(buffer);
                SkASSERT(size == size2);
                
                SkASSERT(tmp3 == tmp);
            }

            rgn.translate(20, 30, &tmp);
            SkASSERT(rgn.isEmpty() || tmp != rgn);
            tmp.translate(-20, -30);
            SkASSERT(tmp == rgn);
        }

        this->drawOrig(canvas, true);

        SkPaint paint;
        paint.setColor((color & ~(0xFF << 24)) | (0x44 << 24));
        paint_rgn(canvas, rgn, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(color);
        paint_rgn(canvas, rgn, paint);
    }
    
    void drawPathOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {
        SkRegion    rgn;
        SkPath      path;

        this->build_rgn(&rgn, op);
        rgn.getBoundaryPath(&path);

        this->drawOrig(canvas, true);

        SkPaint paint;

        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor((color & ~(0xFF << 24)) | (0x44 << 24));
        canvas->drawPath(path, paint);
        paint.setColor(color);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
#ifdef SK_DEBUG
        if (true) {
            SkRegion a, b, c;
            test_union_bug_1505668(&a, &b, &c);
            
            if (false) {    // draw the result of the test
                SkPaint paint;
                
                canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
                paint.setColor(SK_ColorRED);
                paint_rgn(canvas, a, paint);
                paint.setColor(0x800000FF);
                paint_rgn(canvas, b, paint);
                paint.setColor(SK_ColorBLACK);
                paint.setStyle(SkPaint::kStroke_Style);
             //   paint_rgn(canvas, c, paint);
                return;
            }
        }
#endif

        static const struct {
            SkColor         fColor;
            const char*     fName;
            SkRegion::Op    fOp;
        } gOps[] = {
            { SK_ColorBLACK,    "Difference",   SkRegion::kDifference_Op    },
            { SK_ColorRED,      "Intersect",    SkRegion::kIntersect_Op     },
            { 0xFF008800,       "Union",        SkRegion::kUnion_Op         },
            { SK_ColorBLUE,     "XOR",          SkRegion::kXOR_Op           }
        };

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setTextSize(SK_Scalar1*24);

        this->drawOrig(canvas, false);
        canvas->save();
            canvas->translate(SkIntToScalar(200), 0);
            this->drawRgnOped(canvas, SkRegion::kUnion_Op, SK_ColorBLACK);
        canvas->restore();
        
        canvas->translate(0, SkIntToScalar(200));

        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); op++) {
            canvas->drawText(gOps[op].fName, strlen(gOps[op].fName), SkIntToScalar(75), SkIntToScalar(50), textPaint);

            this->drawRgnOped(canvas, gOps[op].fOp, gOps[op].fColor);

            canvas->save();
            canvas->translate(0, SkIntToScalar(200));
            this->drawPathOped(canvas, gOps[op].fOp, gOps[op].fColor);
            canvas->restore();
            
            canvas->translate(SkIntToScalar(200), 0);
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return fRect.contains(SkScalarRound(x), SkScalarRound(y)) ? new Click(this) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        fRect.offset(click->fICurr.fX - click->fIPrev.fX,
                     click->fICurr.fY - click->fIPrev.fY);
        this->inval(NULL);
        return true;
    }
    
private:
    SkIRect    fBase, fRect;
    
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new RegionView; }
static SkViewRegister reg(MyFactory);

