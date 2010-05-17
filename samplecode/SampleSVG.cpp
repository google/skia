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

#include "SkSVGParser.h"

class SVGView : public SkView {
public:    
	SVGView() {
		SkXMLParserError err;
		SkFILEStream stream("/testsvg2.svg");
		SkSVGParser parser(&err);
		if (parser.parse(stream)) {
			const char* text = parser.getFinal();
			SkFILEWStream output("/testanim.txt");
			output.write(text, strlen(text));
		}
	}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("SVG");
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
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) {
        int y = click->fICurr.fY;
        if (y < 0) {
            y = 0;
        } else if (y > 255) {
            y = 255;
        }
        fByte = y;
        this->inval(NULL);
        return true;
    }
    
private:
    int fByte;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SVGView; }
static SkViewRegister reg(MyFactory);

