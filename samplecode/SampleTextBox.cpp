#include "SampleCode.h"
#include "SkView.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkTextBox.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkKey.h"

static const char gText[] = 
	"When in the Course of human events it becomes necessary for one people "
	"to dissolve the political bands which have connected them with another "
	"and to assume among the powers of the earth, the separate and equal "
	"station to which the Laws of Nature and of Nature's God entitle them, "
	"a decent respect to the opinions of mankind requires that they should "
	"declare the causes which impel them to the separation.";

class TextBoxView : public SkView {
public:    
	TextBoxView() {
		fTextSize = SkIntToScalar(32);
	}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("TextBox");
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

		SkScalar margin = 20;
        SkTextBox tbox;
		tbox.setMode(SkTextBox::kLineBreak_Mode);
		tbox.setBox(margin, margin,
					this->width() - margin, this->height() - margin);
		tbox.setSpacing(SkIntToScalar(3)/3, 0);

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setTextSize(fTextSize);

		tbox.draw(canvas, gText, strlen(gText), paint);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
	virtual bool onClick(Click* click) {
		const SkScalar delta = SkIntToScalar(3);
		if (click->fState == Click::kUp_State) {
			if (click->fCurr.fY < this->height()/2) {
				fTextSize += delta;
				this->inval(NULL);
				return true;
			} else {
				if (fTextSize > delta) {
					fTextSize -= delta;
					this->inval(NULL);
					return true;
				}
			}
		}
		return this->INHERITED::onClick(click);
    }
    
private:
    SkScalar fTextSize;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextBoxView; }
static SkViewRegister reg(MyFactory);

