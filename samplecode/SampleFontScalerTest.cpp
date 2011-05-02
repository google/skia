#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkTypeface.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDither.h"

static const struct {
    const char* fName;
    SkTypeface::Style   fStyle;
} gFaces[] = {
    { NULL, SkTypeface::kNormal },
    { NULL, SkTypeface::kBold },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal }
};

static const int gFaceCount = SK_ARRAY_COUNT(gFaces);

class FontScalerTestView : public SampleView {
    SkTypeface* fFaces[gFaceCount];

public:
	FontScalerTestView() {
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i].fName,
                                                   gFaces[i].fStyle);
        }
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~FontScalerTestView() {
        for (int i = 0; i < gFaceCount; i++) {
            SkSafeUnref(fFaces[i]);
        }
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FontScaler Test");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;

        // test handling of obscene cubic values (currently broken)
        if (false) {
            SkPoint pts[4];
            pts[0].set(1.61061274e+09f, 6291456);
            pts[1].set(-7.18397061e+15f, -1.53091184e+13f);
            pts[2].set(-1.30077315e+16f, -2.77196141e+13f);
            pts[3].set(-1.30077315e+16f, -2.77196162e+13f);

            SkPath path;
            path.moveTo(pts[0]);
            path.cubicTo(pts[1], pts[2], pts[3]);
            canvas->drawPath(path, paint);
        }

        canvas->translate(200, 20);
        canvas->rotate(30);

        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        SkSafeUnref(paint.setTypeface(SkTypeface::CreateFromName("Times Roman", SkTypeface::kNormal)));

//        const char* text = "abcdefghijklmnopqrstuvwxyz";
        const char* text = "HnHnHnHnHnHnHnHnH";
        size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(20);

        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            SkRect r;
            r.set(0, 0, x, y*20);
            canvas->drawRect(r, p);
        }

        int index = 0;
        for (int ps = 9; ps <= 24; ps++) {
            textLen = strlen(text);
            paint.setTextSize(SkIntToScalar(ps));
            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(NULL);
            index += 1;
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FontScalerTestView; }
static SkViewRegister reg(MyFactory);

