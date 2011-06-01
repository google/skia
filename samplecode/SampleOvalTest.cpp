#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

static const int kILimit = 101;
static const SkScalar kLimit = SK_Scalar1 * kILimit;

class OvalTestView : public SampleView {
public:
    SkSize      fSize;
    SkPMColor   fInsideColor;   // signals an interior pixel that was not set
    SkPMColor   fOutsideColor;  // signals an exterior pixels that was set
    SkBitmap    fBitmap;

	OvalTestView() {
        fSize.set(SK_Scalar1, SK_Scalar1);

        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, kILimit, kILimit);
        fBitmap.allocPixels();

        fInsideColor = SkPreMultiplyColor(SK_ColorRED);
        fOutsideColor = SkPreMultiplyColor(SK_ColorGREEN);

        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "OvalTest");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawOval() {
        SkCanvas canvas(fBitmap);
        SkPaint p;

        fBitmap.eraseColor(0);
        canvas.drawOval(SkRect::MakeSize(fSize), p);
    }

    int checkOval(int* flatCount, int* buldgeCount) {
        int flatc = 0;
        int buldgec = 0;
        const SkScalar rad = SkScalarHalf(fSize.width());
        SkScalar cx = SkScalarHalf(fSize.width());
        SkScalar cy = SkScalarHalf(fSize.height());
        for (int y = 0; y < kILimit; y++) {
            for (int x = 0; x < kILimit; x++) {
                // measure from pixel centers
                SkScalar px = SkIntToScalar(x) + SK_ScalarHalf;
                SkScalar py = SkIntToScalar(y) + SK_ScalarHalf;

                SkPMColor* ptr = fBitmap.getAddr32(x, y);
                SkScalar dist = SkPoint::Length(px - cx, py - cy);
                if (dist <= rad && !*ptr) {
                    flatc++;
                    *ptr = fInsideColor;
                } else if (dist > rad && *ptr) {
                    buldgec++;
                    *ptr = fOutsideColor;
                }
            }
        }
        if (flatCount) *flatCount = flatc;
        if (buldgeCount) *buldgeCount = buldgec;
        return flatc + buldgec;
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        this->drawOval();
        int flatCount, buldgeCount;
        this->checkOval(&flatCount, &buldgeCount);
        this->inval(NULL);

        canvas->drawBitmap(fBitmap, SkIntToScalar(20), SkIntToScalar(20), NULL);


        static int gFlatCount;
        static int gBuldgeCount;
        gFlatCount += flatCount;
        gBuldgeCount += buldgeCount;

        if (fSize.fWidth < kLimit) {
            SkDebugf("--- width=%g, flat=%d buldge=%d total: flat=%d buldge=%d\n", fSize.fWidth,
                     flatCount, buldgeCount, gFlatCount, gBuldgeCount);
            fSize.fWidth += SK_Scalar1;
            fSize.fHeight += SK_Scalar1;
        } else {
         //   fSize.set(SK_Scalar1, SK_Scalar1);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        return NULL;
    }

private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new OvalTestView; }
static SkViewRegister reg(MyFactory);

