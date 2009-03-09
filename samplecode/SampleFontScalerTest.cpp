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

class FontScalerTestView : public SkView {
    SkTypeface* fFaces[gFaceCount];

public:
	FontScalerTestView() {
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i].fName,
                                                   gFaces[i].fStyle);
        }
    }
    
    virtual ~FontScalerTestView() {
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i]->safeUnref();
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
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTypeface(SkTypeface::CreateFromName("Times Roman", SkTypeface::kNormal))->safeUnref();
        
        const char* text = "HHHaaammmbbbuuurrrgggeeefffooonnnsss";
        const size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(20);

        for (int ps = 9; ps <= 24; ps++) {
            paint.setTextSize(SkIntToScalar(ps));
            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(NULL);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FontScalerTestView; }
static SkViewRegister reg(MyFactory);

